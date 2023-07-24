#pragma once

#include <iostream>
#include <string>
using namespace std;

#include "Expression.cpp"
#include "VectorStack.cpp"
#include "Tools.cpp"
#include "OperatorTable.h"

class Infix {

	VectorStack<Expression> stack;

	private:

		void EvalueChar(char ch) {
			if (Tools::IsNumber(ch) || Tools::IsSeparator(ch) || Tools::IsBlankSpace(ch)) {
				return EvalueDigit(ch);
			}
			if (Tools::IsOperator(ch)) {
				return EvalueOperator(ch);
			}
		}

		void EvalueOperator(char ch) {
			Optional<Expression> previous = stack.Get();
			if (previous.IsSome() && previous.Unwrap()->IsEmpty()) {
				stack.Remove();
			}

			Optional<Expression> right = stack.Remove();
			Optional<Expression> left = stack.Remove();

			Optional<int> actualPrecedence = OperatorTable::GetPrecedence(ch);
			if (actualPrecedence.IsNone()) {
				throw std::runtime_error("Unknown operator " + ch);
			}

			Optional<bool> actualAssociativity = OperatorTable::GetAssociativity(ch);
			if (actualAssociativity.IsNone()) {
				throw std::runtime_error("Unknown operator " + ch);
			}

			VectorQueue<char> leftExpression = EvalueOperatorLeft(left, *actualPrecedence.Unwrap(), *actualAssociativity.Unwrap());
			VectorQueue<char> rightExpression = EvalueOperatorRight(right, *actualPrecedence.Unwrap(), *actualAssociativity.Unwrap());

			Expression ex = Expression(leftExpression, rightExpression, ch);
			stack.Insert(ex);
		}

		VectorQueue<char> EvalueOperatorLeft(Optional<Expression> left, int actualPrecedence, bool actualAssociativity) {
			VectorQueue<char> expression = VectorQueue<char>();
			if (left.IsSome()) {
				expression = left.Unwrap()->GetExpression();
				int leftPrecedence = left.Unwrap()->Precedence();
				if (!actualAssociativity && leftPrecedence <= actualPrecedence) {
					expression = EncapsuleExpression(expression);
				}
			}

			return expression;
		}

		VectorQueue<char> EvalueOperatorRight(Optional<Expression> right, int actualPrecedence, bool actualAssociativity) {
			VectorQueue<char> expression = VectorQueue<char>();
			if (right.IsSome()) {
				expression = right.Unwrap()->GetExpression();
				int rightPrecedence = right.Unwrap()->Precedence();
				if (actualAssociativity && actualPrecedence > rightPrecedence) {
					expression = EncapsuleExpression(expression);
				}
			}
			return expression;
		}

		VectorQueue<char> EncapsuleExpression(VectorQueue<char> expression) {
			Expression aux = Expression();
			aux.Insert('(');
			aux.Insert(expression);
			aux.Insert(')');
			return aux.GetExpression();
		}

		void EvalueDigit(char ch) {
			if (stack.IsEmpty() || Tools::IsBlankSpace(ch)) {
				Expression* ex = new Expression();
				stack.Insert(ex);
			}
			if (!Tools::IsBlankSpace(ch)) {
				Optional<Expression> ex = stack.Get();
				ex.Unwrap()->Insert(ch);
			}
		}

	public:

		Infix() {
			stack = VectorStack<Expression>();
		}

		static Infix Transform(string expression) {
			Infix infix = Infix();
			for (int i = 0; i < expression.size(); i++) {
				char ch = expression[i];
				infix.EvalueChar(ch);
			}
			if (infix.stack.Length() > 1) {
				throw std::runtime_error("Irregular final stack length: " + infix.stack.Length());
			}
			return infix;
		}

		void Print() {
			Optional<Expression> oExpression = stack.Remove();
			if (oExpression.IsSome()) {
				VectorQueue<char> expression = oExpression.Unwrap()->GetExpression();
				Optional<char> character = expression.Remove();
				while (character.IsSome()) {
					cout << *character.Unwrap();
					character = expression.Remove();
					if (character.IsSome() && !Tools::IsSeparator(*character.Unwrap())) {
						cout << " ";
					}
				}
			}
		}

};