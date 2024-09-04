#include "BlockState.h"
#include "../ByteCodeBuilder.h"

BlockState::BlockState(std::vector<std::unique_ptr<State>> states)
	: m_states(std::move(states)) {

}

void BlockState::generate() {
	g_builder.addScope(true);

	try {
		for (auto& state : m_states) {
			state->generate();
		}

		g_builder.deleteScope(true);
	} catch (TerminatorAdded terminator) {
		g_builder.deleteScope(true);
		throw terminator;
	}
}
