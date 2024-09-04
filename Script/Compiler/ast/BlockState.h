#pragma once
#include <vector>
#include <memory>
#include "State.h"

// Statement for built-in instructions
class BlockState final : public State {
private:
	std::vector<std::unique_ptr<State>> m_states;

public:
	BlockState(std::vector<std::unique_ptr<State>> states);

	void generate() override;
};