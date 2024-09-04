#include "ByteCodeBuilder.h"
#include <QFile>
#include <QTextStream>

ByteCodeBuilder g_builder;

Variable g_scopeVar = Variable{ "", Type(), 0, false, true };

// The offset to differentiate between labels and normal positions
constexpr size_t LABELS_OFFSET = 0x1000000;

void ByteCodeBuilder::buildAndStore(QString fileName) {
    replaceLabels();

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
        return;
    
    QTextStream out(&file);
    out << m_imageBuffersCount << "\n"
        << m_maxCallStackSize << "\n"
        << m_maxLocalVariableStackSize << "\n"
        << m_maxStackSize << "\n"
        << m_globalTop << "\n"
        << m_byteCode.size() << "\n";

    for (auto& inst : m_byteCode) {
        out << int(inst.op);
        uint8_t argsCount = getInstructionArgsSize(inst.op);

        for (uint8_t i = 0; i < argsCount; i++) {
            out << " " << inst.value[i];
        }

        out << "\n";
    }
}

size_t ByteCodeBuilder::addInst(Instruction inst) {
    m_byteCode.push_back(std::move(inst));
    return m_byteCode.size() - 1;
}

void ByteCodeBuilder::addBreakInst(uint8_t cycleId) {
    assert(cycleId < m_cycles.size());

    Cycle cycle = m_cycles[m_cycles.size() - cycleId - 1];
    addClearScopeInst(uint32_t(m_localTop - cycle.clearIndex));
    addInst(Instruction(InstructionType::GOTO, (uint32_t)cycle.endLabel));
}

void ByteCodeBuilder::addContinueInst(uint8_t cycleId) {
    assert(cycleId < m_cycles.size());

    Cycle cycle = m_cycles[m_cycles.size() - cycleId - 1];
    addClearScopeInst(uint32_t(m_localTop - cycle.clearIndex));
    addInst(Instruction(InstructionType::GOTO, (uint32_t)cycle.beginLabel));
}

void ByteCodeBuilder::addClearScopeInst(uint32_t totalClear) {
    if (totalClear) {
        addInst(Instruction(InstructionType::CLEAR_SCOPE, totalClear));
    }
}

void ByteCodeBuilder::addRetInst() {
    assert(m_retClearIndices.size());

    addClearScopeInst(uint32_t(m_localTop - m_retClearIndices.back()));
    addInst(Instruction(InstructionType::RET));
}

Cycle* ByteCodeBuilder::addCycle() {
    Cycle cycle;
    cycle.beginLabel = getLabel();
    cycle.endLabel = getLabel();
    cycle.clearIndex = m_localTop;

    m_cycles.push_back(cycle);
    return &m_cycles.back();
}

void ByteCodeBuilder::endCycle() {
    m_cycles.pop_back();
}

void ByteCodeBuilder::addRetClearIndex() {
    m_retClearIndices.push_back(m_localTop);
}

void ByteCodeBuilder::popRetClearIndex() {
    assert(m_retClearIndices.size());

    m_retClearIndices.pop_back();
}

size_t ByteCodeBuilder::getLabel() {
    m_labels.push_back(0);
    return m_labels.size() + LABELS_OFFSET - 1;
}

void ByteCodeBuilder::setLabelAtNextInst(size_t labelId) {
    assert(labelId < m_labels.size() + LABELS_OFFSET);

    m_labels[labelId - LABELS_OFFSET] = m_byteCode.size();
}

void ByteCodeBuilder::replaceLabels() {
    for (auto& inst : m_byteCode) {
        if (inst.op == InstructionType::GOTO || inst.op == InstructionType::GOTO_IF_NOT || inst.op == InstructionType::CALL) {
            if (inst.value[0] >= LABELS_OFFSET) { // It is a label
                assert(inst.value[0] - LABELS_OFFSET < m_labels.size());
                inst.value[0] = m_labels[inst.value[0] - LABELS_OFFSET];
            }
        }
    }

    m_labels.clear();
}

Variable* ByteCodeBuilder::findVariable(const std::string& name) {
    for (int i = m_vars.size() - 1; i >= 0; i--) {
        if (m_vars[i].name == name) {
            return &m_vars[i];
        }
    }

    return nullptr;
}

Function* ByteCodeBuilder::findFunction(const std::string& name, const Type& arguments) {
    for (int i = m_funcs.size() - 1; i >= 0; i--) {
        if (m_funcs[i].name == name && m_funcs[i].arguments == arguments) {
            return &m_funcs[i];
        }
    }

    return nullptr;
}

Function* ByteCodeBuilder::findFunction(size_t funcId) {
    assert(funcId < m_labels.size() + LABELS_OFFSET);

    for (auto& func : m_funcs) {
        if (func.position == funcId) {
            return &func;
        }
    }

    return nullptr;
}

Variable ByteCodeBuilder::addVariable(std::string name, Type type, bool isGlobal) {
    uint32_t& varTop = isGlobal ? m_globalTop : m_localTop;
    uint32_t varSize = getTypeSize(type[0]);

    m_vars.push_back(Variable{ std::move(name), std::move(type), varTop, isGlobal, false });
    varTop += varSize;

    return m_vars.back();
}

size_t ByteCodeBuilder::addFunction(std::string name, Type retType, Type argTypes) {
    m_funcs.push_back(Function{ std::move(name), std::move(argTypes), std::move(retType), getLabel() });

    return m_funcs.back().position;
}

Function& ByteCodeBuilder::setFunctionStartLoc(size_t funcId) {
    setLabelAtNextInst(funcId);

    return *findFunction(funcId);
}

uint32_t ByteCodeBuilder::getCurrentLocalVarLoc(uint32_t index) {
    assert(index <= m_localTop);

    return m_localTop - index;
}

void ByteCodeBuilder::addScope(bool modifyInsts) {
    g_scopeVar.index = m_localTop;
    m_vars.push_back(g_scopeVar);
}

void ByteCodeBuilder::deleteScope(bool modifyInsts) {
    uint32_t totalClear = 0;
    for (int i = m_vars.size() - 1; i >= 0; i--) {
        if (m_vars[i].scopeFlag) {
            m_vars.pop_back();
            break;
        }

        m_localTop -= getTypeSize(m_vars.back().type[0]);
        totalClear += getTypeSize(m_vars.back().type[0]);
        m_vars.pop_back();
    }

    if (modifyInsts) {
        addClearScopeInst(totalClear);
    }
}

void ByteCodeBuilder::setMaxStackSize(uint32_t size) {
    m_maxStackSize = size;
}

void ByteCodeBuilder::setMaxCallStackSize(uint32_t size) {
    m_maxCallStackSize = size;
}

void ByteCodeBuilder::setMaxLocalVariablesStackSize(uint32_t size) {
    m_maxLocalVariableStackSize = size;
}

void ByteCodeBuilder::setImageBuffersCount(uint8_t count) {
    m_imageBuffersCount = count;
}

uint8_t ByteCodeBuilder::getImageBuffersCount() {
    return m_imageBuffersCount;
}
