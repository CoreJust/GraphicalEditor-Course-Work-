#include "CW2VM.h"
#include <QFile>
#include <QTextStream>
#include <QTimer>

CW2VM::CW2VM(ImageEditScene* parent) 
	: m_pScene(parent) {
	m_image = &parent->getImage();
	m_width = m_image->getWidth();
	m_height = m_image->getHeight();
}

CW2VM::~CW2VM() {
    for (Image* img : m_imageBuffers) {
        delete img;
    }
}

utils::Result<Void> CW2VM::loadFromFile(const QString& fileName) {
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        return utils::Failure("Failed to open file: " + fileName);
    }

    if (m_isLoaded) {
        resetVMState();
    }

    QTextStream in(&file);

    uint32_t imageBuffersCount, maxCallStackSize, maxLocalStackSize, maxStackSize, globalsSize, byteCodeSize;
    in >> imageBuffersCount
        >> maxCallStackSize
        >> maxLocalStackSize
        >> maxStackSize
        >> globalsSize
        >> byteCodeSize;

    m_imageBuffers.reserve(imageBuffersCount);
    for (uint32_t i = 0; i < imageBuffersCount; i++) {
        Image *img = new Image();
        if (auto result = img->allocateImage(m_width, m_height); !result.isOk()) {
            return result.extractError();
        }

        m_imageBuffers.push_back(img);
    }

    m_globalVars.resize(globalsSize);
    m_byteCode.reserve(byteCodeSize);

    int op;
    uint32_t value[2];
    for (uint32_t i = 0; i < byteCodeSize; i++) {
        in >> op;
        uint8_t argsCount = getInstructionArgsSize(InstructionType(op));

        for (uint8_t i = 0; i < argsCount; i++) {
            in >> value[i];
        }

        if (argsCount == 0) {
            m_byteCode.push_back(Instruction(InstructionType(op)));
        } else if (argsCount == 1) {
            m_byteCode.push_back(Instruction(InstructionType(op), value[0]));
        } else if (argsCount == 2) {
            m_byteCode.push_back(Instruction(InstructionType(op), value[0], value[1]));
        }
    }

    m_isLoaded = true;
    return utils::Success();
}

void CW2VM::forceHalt() {
    m_shallHalt = true;
}

void CW2VM::execute() {
    while (m_pos < m_byteCode.size()) {
        Instruction& inst = m_byteCode[m_pos++];
        switch (inst.op) {
        case InstructionType::NOPE: break;
        case InstructionType::PUSH:
            m_stack.push_back(*(float*)&inst.value[0]);
            break;
        case InstructionType::PUSH_POINT:
            m_stack.push_back(*(float*)&inst.value[0]);
            m_stack.push_back(*(float*)&inst.value[1]);
            break;
        case InstructionType::POP:
            m_stack.pop_back();
            break;
        case InstructionType::POP_POINT:
            m_stack.pop_back();
            m_stack.pop_back();
            break;
        case InstructionType::POP_COLOR:
            m_stack.pop_back();
            m_stack.pop_back();
            m_stack.pop_back();
            m_stack.pop_back();
            break;
        case InstructionType::GET_COLOR_RED:
            m_stack.pop_back();
            m_stack.pop_back();
            m_stack.pop_back();
            break;
        case InstructionType::GET_COLOR_GREEN: {
            m_stack.pop_back();
            m_stack.pop_back();

            float val = m_stack.back();
            m_stack.pop_back();
            m_stack[m_stack.size() - 1] = val;
            } break;
        case InstructionType::GET_COLOR_BLUE: {
            m_stack.pop_back();

            float val = m_stack.back();
            m_stack.pop_back();
            m_stack.pop_back();
            m_stack[m_stack.size() - 1] = val;
        } break;
        case InstructionType::GET_COLOR_ALPHA: {
            float val = m_stack.back();
            m_stack.pop_back();
            m_stack.pop_back();
            m_stack.pop_back();
            m_stack[m_stack.size() - 1] = val;
        } break;
        case InstructionType::GET_POINT_X:
            m_stack.pop_back();
            break;
        case InstructionType::GET_POINT_Y: {
            float val = m_stack.back();
            m_stack.pop_back();
            m_stack[m_stack.size() - 1] = val;
        } break;
        case InstructionType::ADD_LOCAL: {
            float val = m_stack.back();
            m_stack.pop_back();

            m_localVarStack.push_back(val);
        } break;
        case InstructionType::ADD_LOCAL_POINT: {
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_localVarStack.push_back(val1);
            m_localVarStack.push_back(val2);
        } break;
        case InstructionType::ADD_LOCAL_COLOR: {
            float val4 = m_stack.back();
            m_stack.pop_back();
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_localVarStack.push_back(val1);
            m_localVarStack.push_back(val2);
            m_localVarStack.push_back(val3);
            m_localVarStack.push_back(val4);
        } break;
        case InstructionType::CLEAR_SCOPE: {
            uint32_t totalClear = inst.value[0];
            for (int i = 0; i < totalClear; i++) {
                m_localVarStack.pop_back();
            }
        } break;
        case InstructionType::LOAD: {
            float val = m_localVarStack[m_localVarStack.size() - inst.value[0]];
            m_stack.push_back(val);
        } break;
        case InstructionType::LOAD_POINT: {
            float val1 = m_localVarStack[m_localVarStack.size() - inst.value[0]];
            float val2 = m_localVarStack[m_localVarStack.size() - inst.value[0] + 1];

            m_stack.push_back(val1);
            m_stack.push_back(val2);
        } break;
        case InstructionType::LOAD_COLOR: {
            float val1 = m_localVarStack[m_localVarStack.size() - inst.value[0]];
            float val2 = m_localVarStack[m_localVarStack.size() - inst.value[0] + 1];
            float val3 = m_localVarStack[m_localVarStack.size() - inst.value[0] + 2];
            float val4 = m_localVarStack[m_localVarStack.size() - inst.value[0] + 3];

            m_stack.push_back(val1);
            m_stack.push_back(val2);
            m_stack.push_back(val3);
            m_stack.push_back(val4);
        } break;
        case InstructionType::STORE: {
            float val = m_stack.back();
            m_stack.pop_back();

            m_localVarStack[m_localVarStack.size() - inst.value[0]] = val;
        } break;
        case InstructionType::STORE_POINT: {
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_localVarStack[m_localVarStack.size() - inst.value[0]] = val1;
            m_localVarStack[m_localVarStack.size() - inst.value[0] + 1] = val2;
        } break;
        case InstructionType::STORE_COLOR: {
            float val4 = m_stack.back();
            m_stack.pop_back();
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_localVarStack[m_localVarStack.size() - inst.value[0]] = val1;
            m_localVarStack[m_localVarStack.size() - inst.value[0] + 1] = val2;
            m_localVarStack[m_localVarStack.size() - inst.value[0] + 2] = val3;
            m_localVarStack[m_localVarStack.size() - inst.value[0] + 3] = val4;
        } break;
        case InstructionType::LOAD_GLOBAL: {
            float val = m_globalVars[inst.value[0]];
            m_stack.push_back(val);
        } break;
        case InstructionType::LOAD_POINT_GLOBAL: {
            float val1 = m_globalVars[inst.value[0]];
            float val2 = m_globalVars[inst.value[0] + 1];

            m_stack.push_back(val1);
            m_stack.push_back(val2);
        } break;
        case InstructionType::LOAD_COLOR_GLOBAL: {
            float val1 = m_globalVars[inst.value[0]];
            float val2 = m_globalVars[inst.value[0] + 1];
            float val3 = m_globalVars[inst.value[0] + 2];
            float val4 = m_globalVars[inst.value[0] + 3];

            m_stack.push_back(val1);
            m_stack.push_back(val2);
            m_stack.push_back(val3);
            m_stack.push_back(val4);
        } break;
        case InstructionType::STORE_GLOBAL: {
            float val = m_stack.back();
            m_stack.pop_back();

            m_globalVars[inst.value[0]] = val;
        } break;
        case InstructionType::STORE_POINT_GLOBAL: {
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_globalVars[inst.value[0]] = val1;
            m_globalVars[inst.value[0] + 1] = val2;
        } break;
        case InstructionType::STORE_COLOR_GLOBAL: {
            float val4 = m_stack.back();
            m_stack.pop_back();
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_globalVars[inst.value[0]] = val1;
            m_globalVars[inst.value[0] + 1] = val2;
            m_globalVars[inst.value[0] + 2] = val3;
            m_globalVars[inst.value[0] + 3] = val4;
        } break;
        case InstructionType::ADD: {
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(val1 + val2);
        } break;
        case InstructionType::ADD_POINT: {
            float val4 = m_stack.back();
            m_stack.pop_back();
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(val1 + val3);
            m_stack.push_back(val2 + val4);
        } break;
        case InstructionType::ADD_COLOR: {
            float val8 = m_stack.back();
            m_stack.pop_back();
            float val7 = m_stack.back();
            m_stack.pop_back();
            float val6 = m_stack.back();
            m_stack.pop_back();
            float val5 = m_stack.back();
            m_stack.pop_back();
            float val4 = m_stack.back();
            m_stack.pop_back();
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(val1 + val5);
            m_stack.push_back(val2 + val6);
            m_stack.push_back(val3 + val7);
            m_stack.push_back(val4 + val8);
        } break;
        case InstructionType::ADD_TO_POINT: {
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(val1 + val3);
            m_stack.push_back(val2 + val3);
        } break;
        case InstructionType::ADD_TO_COLOR: {
            float val5 = m_stack.back();
            m_stack.pop_back();
            float val4 = m_stack.back();
            m_stack.pop_back();
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(val1 + val5);
            m_stack.push_back(val2 + val5);
            m_stack.push_back(val3 + val5);
            m_stack.push_back(val4 + val5);
        } break;
        case InstructionType::SUB: {
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(val1 - val2);
        } break;
        case InstructionType::SUB_POINT: {
            float val4 = m_stack.back();
            m_stack.pop_back();
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(val1 - val3);
            m_stack.push_back(val2 - val4);
        } break;
        case InstructionType::SUB_COLOR: {
            float val8 = m_stack.back();
            m_stack.pop_back();
            float val7 = m_stack.back();
            m_stack.pop_back();
            float val6 = m_stack.back();
            m_stack.pop_back();
            float val5 = m_stack.back();
            m_stack.pop_back();
            float val4 = m_stack.back();
            m_stack.pop_back();
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(val1 - val5);
            m_stack.push_back(val2 - val6);
            m_stack.push_back(val3 - val7);
            m_stack.push_back(val4 - val8);
        } break;
        case InstructionType::SUB_FROM_POINT: {
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(val1 - val3);
            m_stack.push_back(val2 - val3);
        } break;
        case InstructionType::SUB_FROM_COLOR: {
            float val5 = m_stack.back();
            m_stack.pop_back();
            float val4 = m_stack.back();
            m_stack.pop_back();
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(val1 - val5);
            m_stack.push_back(val2 - val5);
            m_stack.push_back(val3 - val5);
            m_stack.push_back(val4 - val5);
        } break;
        case InstructionType::MUL: {
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(val1 * val2);
        } break;
        case InstructionType::MUL_POINT_ON_NUMBER: {
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(val1 * val3);
            m_stack.push_back(val2 * val3);
        } break;
        case InstructionType::MUL_COLOR_ON_NUMBER: {
            float val5 = m_stack.back();
            m_stack.pop_back();
            float val4 = m_stack.back();
            m_stack.pop_back();
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(val1 * val5);
            m_stack.push_back(val2 * val5);
            m_stack.push_back(val3 * val5);
            m_stack.push_back(val4 * val5);
        } break;
        case InstructionType::DIV: {
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(val1 / val2);
        } break;
        case InstructionType::DIV_POINT_ON_NUMBER: {
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(val1 / val3);
            m_stack.push_back(val2 / val3);
        } break;
        case InstructionType::DIV_COLOR_ON_NUMBER: {
            float val5 = m_stack.back();
            m_stack.pop_back();
            float val4 = m_stack.back();
            m_stack.pop_back();
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(val1 / val5);
            m_stack.push_back(val2 / val5);
            m_stack.push_back(val3 / val5);
            m_stack.push_back(val4 / val5);
        } break;
        case InstructionType::MOD: {
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(fmod(val1, val2));
        } break;
        case InstructionType::MOD_POINT_ON_NUMBER: {
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(fmod(val1, val3));
            m_stack.push_back(fmod(val2, val3));
        } break;
        case InstructionType::MOD_COLOR_ON_NUMBER: {
            float val5 = m_stack.back();
            m_stack.pop_back();
            float val4 = m_stack.back();
            m_stack.pop_back();
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(fmod(val1, val5));
            m_stack.push_back(fmod(val2, val5));
            m_stack.push_back(fmod(val3, val5));
            m_stack.push_back(fmod(val4, val5));
        } break;
        case InstructionType::POW: {
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(pow(val1, val2));
        } break;
        case InstructionType::POW_POINT_TO_NUMBER: {
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(pow(val1, val3));
            m_stack.push_back(pow(val2, val3));
        } break;
        case InstructionType::POW_COLOR_TO_NUMBER: {
            float val5 = m_stack.back();
            m_stack.pop_back();
            float val4 = m_stack.back();
            m_stack.pop_back();
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(pow(val1, val5));
            m_stack.push_back(pow(val2, val5));
            m_stack.push_back(pow(val3, val5));
            m_stack.push_back(pow(val4, val5));
        } break;
        case InstructionType::NEG: {
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(-val1);
        } break;
        case InstructionType::NEG_POINT: {
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(-val1);
            m_stack.push_back(-val2);
        } break;
        case InstructionType::NEG_COLOR: {
            float val4 = m_stack.back();
            m_stack.pop_back();
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(-val1);
            m_stack.push_back(-val2);
            m_stack.push_back(-val3);
            m_stack.push_back(-val4);
        } break;
        case InstructionType::INC: {
            m_stack[m_stack.size() - 1] += 1;
        } break;
        case InstructionType::DEC: {
            m_stack[m_stack.size() - 1] -= 1;
        } break;
        case InstructionType::NOT: {
            m_stack[m_stack.size() - 1] = (m_stack[m_stack.size() - 1] ? 1 : 0);
        } break;
        case InstructionType::CMP_EQ: {
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(
                (val1 == val2)
                ? 1 : 0
            );
        } break;
        case InstructionType::CMP_EQ_POINTS: {
            float val6 = m_stack.back();
            m_stack.pop_back();
            float val5 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(
                (val1 == val5
                    && val2 == val6)
                ? 1 : 0
            );
        } break;
        case InstructionType::CMP_EQ_COLORS: {
            float val8 = m_stack.back();
            m_stack.pop_back();
            float val7 = m_stack.back();
            m_stack.pop_back();
            float val6 = m_stack.back();
            m_stack.pop_back();
            float val5 = m_stack.back();
            m_stack.pop_back();
            float val4 = m_stack.back();
            m_stack.pop_back();
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(
                (val1 == val5
                && val2 == val6
                && val3 == val7
                && val4 == val8)
                ? 1 : 0
            );
        } break;
        case InstructionType::CMP_NEQ: {
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(
                (val1 != val2)
                ? 1 : 0
            );
        } break;
        case InstructionType::CMP_NEQ_POINTS: {
            float val6 = m_stack.back();
            m_stack.pop_back();
            float val5 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(
                (val1 != val5
                    || val2 != val6)
                ? 1 : 0
            );
        } break;
        case InstructionType::CMP_NEQ_COLORS: {
            float val8 = m_stack.back();
            m_stack.pop_back();
            float val7 = m_stack.back();
            m_stack.pop_back();
            float val6 = m_stack.back();
            m_stack.pop_back();
            float val5 = m_stack.back();
            m_stack.pop_back();
            float val4 = m_stack.back();
            m_stack.pop_back();
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(
                (val1 != val5
                    || val2 != val6
                    || val3 != val7
                    || val4 != val8)
                ? 1 : 0
            );
        } break;
        case InstructionType::CMP_LT: {
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(
                (val1 < val2)
                ? 1 : 0
            );
        } break;
        case InstructionType::CMP_GT: {
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(
                (val1 > val2)
                ? 1 : 0
            );
        } break;
        case InstructionType::CMP_GE: {
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(
                (val1 >= val2)
                ? 1 : 0
            );
        } break;
        case InstructionType::CMP_LE: {
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(
                (val1 <= val2)
                ? 1 : 0
            );
        } break;
        case InstructionType::AND: {
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(
                (val1 && val2)
                ? 1 : 0
            );
        } break;
        case InstructionType::OR: {
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(
                (val1 || val2)
                ? 1 : 0
            );
        } break;
        case InstructionType::GOTO:
            m_pos = inst.value[0];
            break;
        case InstructionType::GOTO_IF_NOT: {
            float val1 = m_stack.back();
            m_stack.pop_back();

            if (!val1) {
                m_pos = inst.value[0];
            }
        } break;
        case InstructionType::CALL: {
            m_callStack.push_back(m_pos);
            m_pos = inst.value[0];
        } break;
        case InstructionType::RET: {
            m_pos = m_callStack.back();
            m_callStack.pop_back();
        } break;
        case InstructionType::HALT: {
            m_pos = m_byteCode.size();
            this->~CW2VM();
            return;
        } break;
        case InstructionType::INIT_RANGE: {
            float step = m_stack.back();
            m_stack.pop_back();
            float to = m_stack.back();
            m_stack.pop_back();
            float from = m_stack.back();
            m_stack.pop_back();

            if (to < from) {
                step = -abs(step);
            } else {
                step = abs(step);
            }

            m_stack.push_back(from);
            m_stack.push_back(to);
            m_stack.push_back(step);
        } break;
        case InstructionType::CHECK_RANGE: {
            float step = m_stack.back();
            m_stack.pop_back();
            float to = m_stack.back();
            m_stack.pop_back();
            float iter = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(step < 0 ? (to < iter ? 1 : 0) : (to > iter ? 1 : 0));
        } break;
        case InstructionType::PUSH_WIDTH: {
            m_stack.push_back(m_width);
        } break;
        case InstructionType::PUSH_HEIGHT: {
            m_stack.push_back(m_height);
        } break;
        case InstructionType::SET_IMAGE: {
            if (inst.value[0] == 0) {
                m_image = &m_pScene->getImage();
            } else {
                m_image = m_imageBuffers[inst.value[0]];
            }
        } break;
        case InstructionType::COPY_IMAGE: {
            Image* from;
            Image* to;

            if (inst.value[0] == 0) {
                from = &m_pScene->getImage();
            } else {
                from = m_imageBuffers[inst.value[0]];
            }

            if (inst.value[1] == 0) {
                to = &m_pScene->getImage();
            } else {
                to = m_imageBuffers[inst.value[1]];
            }

            if (from != to) {
                from->copyFrom(*to);
            }
        } break;
        case InstructionType::UPDATE:
            m_pScene->update();
            m_width = m_pScene->getImage().getWidth();
            m_height = m_pScene->getImage().getHeight();
            break;
        case InstructionType::SET_COLOR: {
            float val4 = m_stack.back();
            m_stack.pop_back();
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            uint8_t r = utils::clamp(int(val1), 0, 255);
            uint8_t g = utils::clamp(int(val2), 0, 255);
            uint8_t b = utils::clamp(int(val3), 0, 255);
            uint8_t a = utils::clamp(int(val4), 0, 255);

            m_color = Color(r, g, b, a);
        } break;
        case InstructionType::SET_WIDTH: {
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_toolWidth = val1;
        } break;
        case InstructionType::DRAW_PIX: {
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            uint32_t fromX = utils::clamp(int(val1), 0, m_width);
            uint32_t fromY = utils::clamp(int(val2), 0, m_height);

            m_image->drawXStroke(fromX, fromY, 1, m_color);
        } break;
        case InstructionType::DRAW_STROKE: {
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            uint32_t fromX = utils::clamp(int(val1), 0, m_width);
            uint32_t fromY = utils::clamp(int(val2), 0, m_height);
            uint32_t length = utils::clamp(int(val3), 0, m_width - fromX + 1);

            m_image->drawXStroke(fromX, fromY, length, m_color);
        } break;
        case InstructionType::DRAW_LINE: {
            float val4 = m_stack.back();
            m_stack.pop_back();
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            int fromX = utils::clamp(int(val1), 0, m_width);
            int fromY = utils::clamp(int(val2), 0, m_height);
            int toX = utils::clamp(int(val3), 0, m_width);
            int toY = utils::clamp(int(val4), 0, m_height);

            m_image->drawLine(QPoint(fromX, fromY), QPoint(toX, toY), m_toolWidth, m_color);
        } break;
        case InstructionType::DRAW_RECT: {
            float val4 = m_stack.back();
            m_stack.pop_back();
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            int fromX = utils::clamp(int(val1), 0, m_width);
            int fromY = utils::clamp(int(val2), 0, m_height);
            int width = utils::clamp(int(val3), 0, m_width - fromX);
            int height = utils::clamp(int(val4), 0, m_height - fromY);

            m_image->drawRect(QPoint(fromX, fromY), width, height, m_toolWidth, m_color);
        } break;
        case InstructionType::DRAW_CIRCLE: {
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            int fromX = utils::clamp(int(val1), 0, m_width);
            int fromY = utils::clamp(int(val2), 0, m_height);

            m_image->drawCircle(QPointF(fromX, fromY), val3, m_toolWidth, m_color);
        } break;
        case InstructionType::FILL_RECT: {
            float val4 = m_stack.back();
            m_stack.pop_back();
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            int fromX = utils::clamp(int(val1), 0, m_width);
            int fromY = utils::clamp(int(val2), 0, m_height);
            int width = utils::clamp(int(val3), 0, m_width - fromX);
            int height = utils::clamp(int(val4), 0, m_height - fromY);

            m_image->fillRect(QPoint(fromX, fromY), width, height, m_color);
        } break;
        case InstructionType::FILL_CIRCLE: {
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            int fromX = utils::clamp(int(val1), 0, m_width);
            int fromY = utils::clamp(int(val2), 0, m_height);

            m_image->fillCircle(QPointF(fromX, fromY), val3, m_color);
        } break;
        case InstructionType::SLEEP: {
            float val1 = m_stack.back();
            m_stack.pop_back();

            QTimer::singleShot(int(val1), this, &CW2VM::delayedExecute);
            return;
        } break;
        case InstructionType::ABS: {
            m_stack[m_stack.size() - 1] = fabs(m_stack[m_stack.size() - 1]);
        } break;
        case InstructionType::MIN: {
            uint32_t cnt = inst.value[0];

            float minVal = m_stack.back();
            m_stack.pop_back();

            for (int i = 1; i < cnt; i++) {
                if (m_stack.back() < minVal) {
                    minVal = m_stack.back();
                }

                m_stack.pop_back();
            }

            m_stack.push_back(minVal);
        } break;
        case InstructionType::MAX: {
            uint32_t cnt = inst.value[0];

            float maxVal = m_stack.back();
            m_stack.pop_back();

            for (int i = 1; i < cnt; i++) {
                if (m_stack.back() > maxVal) {
                    maxVal = m_stack.back();
                }

                m_stack.pop_back();
            }

            m_stack.push_back(maxVal);
        } break;
        case InstructionType::SUM: {
            uint32_t cnt = inst.value[0];

            float sumVal = m_stack.back();
            m_stack.pop_back();

            for (int i = 1; i < cnt; i++) {
                sumVal += m_stack.back();
                m_stack.pop_back();
            }

            m_stack.push_back(sumVal);
        } break;
        case InstructionType::ROUND: {
            m_stack[m_stack.size() - 1] = round(m_stack[m_stack.size() - 1]);
        } break;
        case InstructionType::FLOOR: {
            m_stack[m_stack.size() - 1] = floor(m_stack[m_stack.size() - 1]);
        } break;
        case InstructionType::CEIL: {
            m_stack[m_stack.size() - 1] = ceil(m_stack[m_stack.size() - 1]);
        } break;
        case InstructionType::SIN: {
            m_stack[m_stack.size() - 1] = sin(m_stack[m_stack.size() - 1]);
        } break;
        case InstructionType::COS: {
            m_stack[m_stack.size() - 1] = cos(m_stack[m_stack.size() - 1]);
        } break;
        case InstructionType::TAN: {
            m_stack[m_stack.size() - 1] = tan(m_stack[m_stack.size() - 1]);
        } break;
        case InstructionType::COT: {
            m_stack[m_stack.size() - 1] = 1 / tan(m_stack[m_stack.size() - 1]);
        } break;
        case InstructionType::EXP: {
            m_stack[m_stack.size() - 1] = exp(m_stack[m_stack.size() - 1]);
        } break;
        case InstructionType::LOG: {
            m_stack[m_stack.size() - 1] = log(m_stack[m_stack.size() - 1]);
        } break;
        case InstructionType::LENGTH: {
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            m_stack.push_back(sqrt(val1 * val1 + val2 * val2));
        } break;
        case InstructionType::DISTANCE: {
            float val4 = m_stack.back();
            m_stack.pop_back();
            float val3 = m_stack.back();
            m_stack.pop_back();
            float val2 = m_stack.back();
            m_stack.pop_back();
            float val1 = m_stack.back();
            m_stack.pop_back();

            float x = val1 - val3;
            float y = val2 - val4;

            m_stack.push_back(sqrt(x * x + y * y));
        } break;
        default:
            break;
        }
    }

    this->~CW2VM();
}

void CW2VM::delayedExecute() {
    if (m_shallHalt) {
        m_shallHalt = false;
    } else {
        execute();
    }
}

void CW2VM::resetVMState() {
    m_pos = 0;
    m_byteCode.clear();
    m_callStack.clear();
    m_globalVars.clear();
    m_stack.clear();
    m_localVarStack.clear();
    m_color = Color::Black;
    m_toolWidth = 3;

    for (Image* img : m_imageBuffers) {
        delete img;
    }

    m_imageBuffers.clear();
}
