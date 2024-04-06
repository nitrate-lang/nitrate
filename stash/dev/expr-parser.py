#!/usr/bin/env python

import json


class Token():
    def __init__(self, type, value):
        self.type = type
        self.value = value

    def __str__(self):
        return f'Token({self.type}, {self.value})'

    def __repr__(self):
        return self.__str__()


class Scanner():
    # Tokens are already lexex. We will just simulate the scanner
    def __init__(self, tokens):
        self.tokens = tokens
        self.current = 0

    def next(self):
        if self.current < len(self.tokens):
            token = self.tokens[self.current]
            self.current += 1
            return token
        return None

    def peek(self):
        if self.current < len(self.tokens):
            return self.tokens[self.current]
        return None

    def __str__(self):
        return f'Scanner({self.tokens})'


class Node():
    def __init__(self):
        pass

    def __str__(self):
        return f'Node()'


class BinaryNode(Node):
    def __init__(self, op, left, right):
        self.op = op
        self.left = left
        self.right = right

    def __str__(self):
        return f'Node({self.op}, {self.left}, {self.right})'


class UnaryNode(Node):
    def __init__(self, op, right):
        self.op = op
        self.right = right

    def __str__(self):
        return f'Node({self.op}, {self.right})'


class Literal(Node):
    def __init__(self, value):
        self.value = value

    def __str__(self):
        return f'Literal({self.value})'

    def __repr__(self):
        return self.__str__()


def precedence(op):
    precedence_table = {
        '=': 0,
        '+': 1,
        '-': 1,
        '*': 2,
        '/': 2,
    }
    return precedence_table.get(op, -1)


def parse_expr(scanner: Scanner) -> Node:
    output_queue = []
    operator_stack = []

    while True:
        tok = scanner.peek()
        if tok is None:
            break

        if tok.type == 'Punctuation' and tok.value == ';':
            break

        scanner.next()

        if tok.type == 'Literal':
            output_queue.append(Literal(tok.value))

        elif tok.type == 'Punctuation' and tok.value == '(':
            r = parse_expr(scanner)
            if r is None:
                return None
            output_queue.append(r)

        elif tok.type == 'Punctuation' and tok.value == ')':
            while operator_stack and operator_stack[-1].type != 'Punctuation' and operator_stack[-1].value != '(':
                output_queue.append(operator_stack.pop())
            if operator_stack and operator_stack[-1].type == 'Punctuation' and operator_stack[-1].value == '(':
                operator_stack.pop()  # Discard the left parenthesis
            else:
                return None

        elif tok.type == 'Operator':
            while operator_stack and precedence(operator_stack[-1].value) >= precedence(tok.value):
                output_queue.append(operator_stack.pop())
            operator_stack.append(tok)

    while operator_stack:
        output_queue.append(operator_stack.pop())

    return build_ast(output_queue)


def build_ast(output_queue):
    stack = []

    for token in output_queue:
        if isinstance(token, Literal):
            stack.append(token)
        elif isinstance(token, Token):
            if token.type == 'Operator':
                if len(stack) < 2:
                    return None
                right = stack.pop()
                left = stack.pop()
                if token.value == '=':
                    stack.append(BinaryNode(token.value, left, right))
                else:
                    stack.append(BinaryNode(token.value, left, right))
            elif isinstance(token, Node):
                stack.append(token)

    if len(stack) != 1:
        return None

    return stack[0]


def main():
    tokens = [Token('Literal', 4),
              Token('Operator', '+'),
              Token('Literal', 4)]
    scanner = Scanner(tokens)

    ast = parse_expr(scanner)
    print(ast)


if __name__ == '__main__':
    main()
