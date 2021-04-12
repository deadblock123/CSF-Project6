#include <map>
#include <string>
#include <cstddef>
#include <vector>
#include <sstream>
#include <iostream>
#include "calc.h"
/*bool onlyAlphabetic (std::string input);
int main () {
	if (onlyAlphabetic (std::string ("1bcbdb"))) {
		std::cout << "true";
	}
	std::cout << "false";
	std::cout << onlyAlphabetic (std::string (".bcbdb"));
	return 0;
}*/

std::vector<std::string> tokenize(const std::string &expr) {
        std::vector<std::string> vec;
        std::stringstream s(expr);

        std::string tok;
        while (s >> tok) {
                vec.push_back(tok);
        }

        return vec;
}

bool onlyElementsOf (std::string validCharacters, std::string input) {
        std::size_t found = input.find_first_not_of(validCharacters);

        if (found != std::string::npos) {
                return 0;
        }

        return 1;
}

bool onlyAlphabetic (std::string input) {
        return onlyElementsOf(std::string("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"), input);
}

bool onlyNumeric (std::string input) {
        return onlyElementsOf(std::string("0123456789"), input);
}

bool onlyEquals (std::string input) {
        if (input.size() != 1) {
                return 0;
        }
        return onlyElementsOf(std::string("="), input);
}

bool onlyOperations (std::string input) {
        if (input.size() != 1) {
                return 0;
        }
        return onlyElementsOf(std::string("+-*/"), input);
}


struct Calc {
private:
	std::map<std::string, int> variableTable;	

public:
	Calc();
	~Calc();
	int evalExpr(const std::string &expr, int &result);

private:
	int evaluate (int left, int right, std::string op);
	int operandValue (std::string operand);
	bool checkOperand (std::string operand);
	bool checkOperation (std::string operation);
	bool checkVariable (std::string variable);
	bool checkEquals (std::string variable);
	int validateCommand (std::vector<std::string> tokens);
};

extern "C" struct Calc *calc_create(void) {
	return new Calc();
}

extern "C" void calc_destroy(struct Calc *calc) {
	delete calc;
}

extern "C" int calc_eval(struct Calc *calc, const char *expr, int *result) {
	return calc->evalExpr(expr, *result);
}

Calc::Calc() {
}

Calc::~Calc() {
}

int Calc::evalExpr(const std::string &expr, int &result) {
	std::vector<std::string> tokens = tokenize(expr);
	
	int caseNumber = validateCommand(tokens);
	if (caseNumber == 1) {
		result = operandValue(tokens.at(0));
		return 1;
	}
	else if (caseNumber == 2 ) {
		result = evaluate(operandValue(tokens.at(0)), operandValue(tokens.at(2)), tokens.at(1));
		return 1;
	}
	else if (caseNumber == 3) {
		result = operandValue(tokens.at(2));
		variableTable[tokens.at(0)] = result;
		return 1;
	}
	else if (caseNumber == 4) {
		result = evaluate(operandValue(tokens.at(2)), operandValue(tokens.at(4)), tokens.at(3));
		variableTable[tokens.at(0)] = result;
		return 1;
	}

	return 0;
}

int Calc::evaluate (int left, int right, std::string op) {
	if (op.compare("+") == 0) {
		return left + right;
	}
	else if (op.compare("-") == 0) {
		return left - right;
	}
	else if (op.compare("*") == 0) {
		return left * right;
	}
	else if (op.compare("/") == 0) {
		return left / right;
	}
	
	//should not reach this point, here for debugging
	return -123456;
}

int Calc::validateCommand (std::vector<std::string> tokens) {
        if (tokens.size() == 1) {
                if (checkOperand(tokens.at(0))) return 1;
        }
        if (tokens.size() == 3) {
                if (tokens.at(1).compare("/") == 0 && tokens.at(2).compare("0") == 0) {
			return -1;
		}
                if (checkOperand(tokens.at(0)) && checkOperation(tokens.at(1)) && checkOperand(tokens.at(2))) return 2;
	}
        if (tokens.size() == 3) {
                if (checkVariable(tokens.at(0)) && checkEquals(tokens.at(1)) && checkOperand(tokens.at(2))) return 3;
        }
        if (tokens.size() == 5) {
                if (tokens.at(3).compare("/") == 0 && tokens.at(4).compare("0") == 0) {
			return -1;
		}
                if (checkVariable(tokens.at(0)) && checkEquals(tokens.at(1)) && checkOperand(tokens.at(2)) && checkOperation(tokens.at(3)) && checkOperand(tokens.at(4))) return 4;
        }

        return -1;
}

int Calc::operandValue (std::string operand) {
	if (onlyNumeric(operand)) {
		return std::stoi(operand);
	}

	return variableTable[operand];
}

bool Calc::checkOperand (std::string operand) {
	if (onlyNumeric(operand)) {
		return 1;
	}

	else if (onlyAlphabetic(operand)) {
		if (variableTable.find(operand) == variableTable.end()) {
			return 0;
		}
		return 1;
	}

	return 0;
}

bool Calc::checkOperation (std::string operation) {
	if (operation.size() != 1) {
		return 0;
	}

	if (onlyOperations(operation)) {
		return 1;
	}

	return 0;
}

bool Calc::checkVariable (std::string variable) {
	return onlyAlphabetic(variable);
}

bool Calc::checkEquals (std::string equals) {
	if (equals.size() != 1) {
		return 0;
	}

	return onlyEquals(equals);
}
