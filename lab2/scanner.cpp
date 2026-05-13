#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>

using namespace std;

struct Token {
    string type;
    string value;
    int line;
    int column;
};

unordered_map<string, string> keywords = {
    {"int", "INT"},
    {"float", "FLOAT"},
    {"void", "VOID"},
    {"if", "IF"},
    {"else", "ELSE"},
    {"while", "WHILE"},
    {"return", "RETURN"},
    {"input", "INPUT"},
    {"print", "PRINT"}
};

string tokenToString(const Token& token) {
    if (token.value == "-") {
        return "(" + token.type + ", -)";
    }
    return "(" + token.type + ", " + token.value + ")";
}

bool isLetterOrUnderline(char c) {
    return isalpha((unsigned char)c) || c == '_';
}

bool isLetterDigitOrUnderline(char c) {
    return isalnum((unsigned char)c) || c == '_';
}

class Scanner {
private:
    string source;
    int pos;
    int line;
    int column;
    vector<Token> tokens;
    vector<string> errors;

    char peek(int offset = 0) {
        if (pos + offset >= (int)source.size()) {
            return '\0';
        }
        return source[pos + offset];
    }

    char advance() {
        char c = peek();
        pos++;

        if (c == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }

        return c;
    }

    void addToken(const string& type, const string& value, int startLine, int startColumn) {
        tokens.push_back({type, value, startLine, startColumn});
    }

    void addError(const string& message, int errLine, int errColumn) {
        stringstream ss;
        ss << "Lexical Error at line " << errLine << ", column " << errColumn << ": " << message;
        errors.push_back(ss.str());
    }

    void skipWhitespace() {
        while (isspace((unsigned char)peek())) {
            advance();
        }
    }

    void skipLineComment() {
        while (peek() != '\n' && peek() != '\0') {
            advance();
        }
    }

    void skipBlockComment() {
        int startLine = line;
        int startColumn = column;

        advance(); // /
        advance(); // *

        while (peek() != '\0') {
            if (peek() == '*' && peek(1) == '/') {
                advance();
                advance();
                return;
            }
            advance();
        }

        addError("Unclosed block comment", startLine, startColumn);
    }

    void scanIdentifierOrKeyword() {
        int startLine = line;
        int startColumn = column;
        string lexeme;

        while (isLetterDigitOrUnderline(peek())) {
            lexeme += advance();
        }

        if (keywords.count(lexeme)) {
            addToken(keywords[lexeme], lexeme, startLine, startColumn);
        } else {
            addToken("ID", lexeme, startLine, startColumn);
        }
    }

    void scanNumber() {
        int startLine = line;
        int startColumn = column;
        string lexeme;
        bool hasDot = false;

        while (isdigit((unsigned char)peek())) {
            lexeme += advance();
        }

        if (peek() == '.') {
            hasDot = true;
            lexeme += advance();

            if (!isdigit((unsigned char)peek())) {
                addError("Invalid float number: " + lexeme, startLine, startColumn);
                return;
            }

            while (isdigit((unsigned char)peek())) {
                lexeme += advance();
            }
        }

        if (hasDot) {
            addToken("FLO", lexeme, startLine, startColumn);
        } else {
            addToken("INT", lexeme, startLine, startColumn);
        }
    }

    void scanOperatorOrDelimiter() {
        int startLine = line;
        int startColumn = column;
        char c = peek();

        if (c == '+') {
            if (peek(1) == '+') {
                advance();
                advance();
                addToken("AAA", "++", startLine, startColumn);
            } else {
                advance();
                addToken("ADD", "+", startLine, startColumn);
            }
        }
        else if (c == '-') {
            if (peek(1) == '-') {
                advance();
                advance();
                addToken("AAS", "--", startLine, startColumn);
            } else {
                advance();
                addToken("ADD", "-", startLine, startColumn);
            }
        }
        else if (c == '*') {
            advance();
            addToken("MUL", "*", startLine, startColumn);
        }
        else if (c == '/') {
            if (peek(1) == '/') {
                skipLineComment();
            }
            else if (peek(1) == '*') {
                skipBlockComment();
            }
            else {
                advance();
                addToken("MUL", "/", startLine, startColumn);
            }
        }
        else if (c == '<') {
            advance();
            if (peek() == '=') {
                advance();
                addToken("ROP", "<=", startLine, startColumn);
            } else {
                addToken("ROP", "<", startLine, startColumn);
            }
        }
        else if (c == '>') {
            advance();
            if (peek() == '=') {
                advance();
                addToken("ROP", ">=", startLine, startColumn);
            } else {
                addToken("ROP", ">", startLine, startColumn);
            }
        }
        else if (c == '=') {
            advance();
            if (peek() == '=') {
                advance();
                addToken("ROP", "==", startLine, startColumn);
            } else {
                addToken("ASG", "=", startLine, startColumn);
            }
        }
        else if (c == '!') {
            advance();
            if (peek() == '=') {
                advance();
                addToken("ROP", "!=", startLine, startColumn);
            } else {
                addError("Unexpected character '!'. Did you mean '!='?", startLine, startColumn);
            }
        }
        else if (c == '(') {
            advance();
            addToken("LPA", "(", startLine, startColumn);
        }
        else if (c == ')') {
            advance();
            addToken("RPA", ")", startLine, startColumn);
        }
        else if (c == '[') {
            advance();
            addToken("LBK", "[", startLine, startColumn);
        }
        else if (c == ']') {
            advance();
            addToken("RBK", "]", startLine, startColumn);
        }
        else if (c == '{') {
            advance();
            addToken("LBR", "{", startLine, startColumn);
        }
        else if (c == '}') {
            advance();
            addToken("RBR", "}", startLine, startColumn);
        }
        else if (c == ',') {
            advance();
            addToken("CMA", ",", startLine, startColumn);
        }
        else if (c == ';') {
            advance();
            addToken("SCO", ";", startLine, startColumn);
        }
        else {
            string msg = "Unknown character: ";
            msg += c;
            addError(msg, startLine, startColumn);
            advance();
        }
    }

public:
    Scanner(const string& input) {
        source = input;
        pos = 0;
        line = 1;
        column = 1;
    }

    void scan() {
        while (peek() != '\0') {
            skipWhitespace();

            if (peek() == '\0') {
                break;
            }

            if (isLetterOrUnderline(peek())) {
                scanIdentifierOrKeyword();
            }
            else if (isdigit((unsigned char)peek())) {
                scanNumber();
            }
            else {
                scanOperatorOrDelimiter();
            }
        }
    }

    vector<Token> getTokens() {
        return tokens;
    }

    vector<string> getErrors() {
        return errors;
    }
};

string readFile(const string& filename) {
    ifstream fin(filename);
    if (!fin.is_open()) {
        return "";
    }

    stringstream buffer;
    buffer << fin.rdbuf();
    fin.close();

    return buffer.str();
}

void writeTokensToFile(const vector<Token>& tokens, const string& filename) {
    ofstream fout(filename);

    for (const Token& token : tokens) {
        fout << tokenToString(token) << endl;
    }

    fout.close();
}

void printTokens(const vector<Token>& tokens) {
    for (const Token& token : tokens) {
        cout << tokenToString(token) << endl;
    }
}

void printErrors(const vector<string>& errors) {
    for (const string& err : errors) {
        cerr << err << endl;
    }
}

string classifySingleWord(const string& word) {
    Scanner scanner(word);
    scanner.scan();
    vector<Token> tokens = scanner.getTokens();

    if (tokens.size() == 1 && scanner.getErrors().empty()) {
        return tokens[0].type;
    }

    return "ERROR";
}

int main() {
    cout << "Please choose mode:" << endl;
    cout << "1: classify several strings" << endl;
    cout << "2: scan one line of source code" << endl;
    cout << "3: scan source file test.c and output token_output.txt" << endl;

    int mode;
    cin >> mode;

    if (mode == 1) {
        int n;
        cin >> n;

        for (int i = 0; i < n; i++) {
            string word;
            cin >> word;
            cout << classifySingleWord(word) << endl;
        }
    }
    else if (mode == 2) {
        cin.ignore();

        string lineText;
        getline(cin, lineText);

        Scanner scanner(lineText);
        scanner.scan();

        vector<Token> tokens = scanner.getTokens();
        vector<string> errors = scanner.getErrors();

        printTokens(tokens);
        printErrors(errors);
    }
    else if (mode == 3) {
        string code = readFile("test.c");

        if (code.empty()) {
            cout << "Cannot open test.c or file is empty." << endl;
            return 0;
        }

        Scanner scanner(code);
        scanner.scan();

        vector<Token> tokens = scanner.getTokens();
        vector<string> errors = scanner.getErrors();

        printTokens(tokens);
        printErrors(errors);

        writeTokensToFile(tokens, "token_output.txt");

        cout << "Token output has been written to token_output.txt" << endl;
    }
    else {
        cout << "Invalid mode." << endl;
    }

    return 0;
}