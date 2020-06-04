#include "peeholeMatching.h"
#include "RISCVassembly.h"
#include "RISCVinstruction.h"
#include "RISCVOperand.h"

void PeeholeMatchingOptimizer::run()
{
	omitTrivialMoves();
}

void PeeholeMatchingOptimizer::omitTrivialMoves()
{
	for(auto &f : program->getFunctions())
		for(auto &b : f->getBlockList())
			for (auto i = b->getFront(); i != nullptr;) 
				if (i->category() == RISCVinstruction::MOV) {
					auto move = std::static_pointer_cast<MoveAssembly>(i);
					i = i->getNextInstr();
					if (move->getRd() == move->getRs1()) removeRISCVinstruction(move);
				}
				else i = i->getNextInstr();
}
