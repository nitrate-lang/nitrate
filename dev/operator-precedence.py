def precedence(operator):
    if operator in ('+', '-'):
        return 1
    elif operator in ('*', '/'):
        return 2
    elif operator == '=':
        return 0
    else:
        return -1  # Unknown operator

def parse_expression(expression):
    tokens = expression.split()
    output = []
    operator_stack = []

    for token in tokens:
        if token.isdigit():  # Operand
            output.append(token)
        elif token in ('+', '-', '*', '/', '='):  # Operator
            while operator_stack and precedence(operator_stack[-1]) >= precedence(token):
                output.append(operator_stack.pop())
            operator_stack.append(token)
        elif token == '(':  # Left parenthesis
            operator_stack.append(token)
        elif token == ')':  # Right parenthesis
            while operator_stack and operator_stack[-1] != '(':
                output.append(operator_stack.pop())
            operator_stack.pop()  # Discard the left parenthesis

    while operator_stack:
        output.append(operator_stack.pop())

    return output

def evaluate_expression(expression):
    stack = []
    for token in expression:
        if token.isdigit():
            stack.append(int(token))
        elif token == '=':
            value = stack.pop()
            var_name = stack.pop()
            variables[var_name] = value
        else:
            operand2 = stack.pop()
            operand1 = stack.pop()
            if token == '+':
                stack.append(operand1 + operand2)
            elif token == '-':
                stack.append(operand1 - operand2)
            elif token == '*':
                stack.append(operand1 * operand2)
            elif token == '/':
                stack.append(operand1 / operand2)
    return stack.pop()

# Example usage:
expression = "3 + 5 * 2 = x"
variables = {}
parsed_expression = parse_expression(expression)
result = evaluate_expression(parsed_expression)
print("Result:", result)
print("Variables:", variables)
