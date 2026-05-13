#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <algorithm>

using namespace std;

struct DFA {
    set<char> alphabet;
    set<string> states;
    string startState;
    set<string> acceptStates;
    map<pair<string, char>, string> transitions;
};

vector<string> split(const string& s) {
    vector<string> result;
    stringstream ss(s);
    string token;
    while (ss >> token) {
        result.push_back(token);
    }
    return result;
}

bool loadDFA(const string& filename, DFA& dfa) {
    ifstream fin(filename);
    if (!fin.is_open()) {
        cout << "无法打开文件: " << filename << endl;
        return false;
    }

    string line;
    bool readingTransitions = false;

    while (getline(fin, line)) {
        if (line.empty()) continue;

        if (line.find("alphabet:") == 0) {
            vector<string> tokens = split(line.substr(9));
            for (auto& t : tokens) {
                if (t.size() != 1) {
                    cout << "字符集中的元素必须是单个字符。" << endl;
                    return false;
                }
                dfa.alphabet.insert(t[0]);
            }
        }
        else if (line.find("states:") == 0) {
            vector<string> tokens = split(line.substr(7));
            for (auto& t : tokens) {
                dfa.states.insert(t);
            }
        }
        else if (line.find("start:") == 0) {
            vector<string> tokens = split(line.substr(6));
            if (tokens.size() != 1) {
                cout << "开始状态必须唯一。" << endl;
                return false;
            }
            dfa.startState = tokens[0];
        }
        else if (line.find("accept:") == 0) {
            vector<string> tokens = split(line.substr(7));
            for (auto& t : tokens) {
                dfa.acceptStates.insert(t);
            }
        }
        else if (line.find("transitions:") == 0) {
            readingTransitions = true;
        }
        else if (readingTransitions) {
            vector<string> tokens = split(line);
            if (tokens.size() != 3) {
                cout << "状态转移格式错误: " << line << endl;
                return false;
            }

            string from = tokens[0];
            string symbolStr = tokens[1];
            string to = tokens[2];

            if (symbolStr.size() != 1) {
                cout << "输入符号必须是单个字符。" << endl;
                return false;
            }

            char symbol = symbolStr[0];
            dfa.transitions[{from, symbol}] = to;
        }
    }

    fin.close();
    return true;
}

bool validateDFA(const DFA& dfa) {
    bool ok = true;

    if (dfa.alphabet.empty()) {
        cout << "错误：字符集为空。" << endl;
        ok = false;
    }

    if (dfa.states.empty()) {
        cout << "错误：状态集为空。" << endl;
        ok = false;
    }

    if (dfa.startState.empty()) {
        cout << "错误：开始状态为空。" << endl;
        ok = false;
    }
    else if (dfa.states.find(dfa.startState) == dfa.states.end()) {
        cout << "错误：开始状态不在状态集中。" << endl;
        ok = false;
    }

    if (dfa.acceptStates.empty()) {
        cout << "错误：接受状态集为空。" << endl;
        ok = false;
    }
    else {
        for (auto& st : dfa.acceptStates) {
            if (dfa.states.find(st) == dfa.states.end()) {
                cout << "错误：接受状态 " << st << " 不在状态集中。" << endl;
                ok = false;
            }
        }
    }

    for (auto& st : dfa.states) {
        for (auto c : dfa.alphabet) {
            auto it = dfa.transitions.find({st, c});
            if (it == dfa.transitions.end()) {
                cout << "错误：缺少转移函数 δ(" << st << ", " << c << ")" << endl;
                ok = false;
            }
            else if (dfa.states.find(it->second) == dfa.states.end()) {
                cout << "错误：转移目标状态 " << it->second << " 不在状态集中。" << endl;
                ok = false;
            }
        }
    }

    return ok;
}

void printDFAInfo(const DFA& dfa) {
    cout << "\n===== DFA 基本信息 =====" << endl;

    cout << "字符集: ";
    for (auto c : dfa.alphabet) cout << c << " ";
    cout << endl;

    cout << "状态集: ";
    for (auto s : dfa.states) cout << s << " ";
    cout << endl;

    cout << "开始状态: " << dfa.startState << endl;

    cout << "接受状态集: ";
    for (auto s : dfa.acceptStates) cout << s << " ";
    cout << endl;
}

bool runDFA(const DFA& dfa, const string& input, bool showProcess = true) {
    string current = dfa.startState;

    if (showProcess) {
        cout << "\n===== DFA 识别过程 =====" << endl;
        cout << "开始状态: " << current << endl;
    }

    for (char c : input) {
        if (dfa.alphabet.find(c) == dfa.alphabet.end()) {
            cout << "输入字符 '" << c << "' 不在字符集中，字符串非法。" << endl;
            return false;
        }

        auto it = dfa.transitions.find({current, c});
        if (it == dfa.transitions.end()) {
            cout << "不存在转移: δ(" << current << ", " << c << ")" << endl;
            return false;
        }

        if (showProcess) {
            cout << "读入字符 " << c << " : " << current << " -> " << it->second << endl;
        }
        current = it->second;
    }

    if (showProcess) {
        cout << "结束状态: " << current << endl;
    }

    bool accepted = (dfa.acceptStates.find(current) != dfa.acceptStates.end());

    if (showProcess) {
        if (accepted)
            cout << "结果：该字符串属于 DFA 的语言。" << endl;
        else
            cout << "结果：该字符串不属于 DFA 的语言。" << endl;
    }

    return accepted;
}

void generateStrings(const DFA& dfa, string current, int maxLen, vector<string>& results) {
    if ((int)current.size() > maxLen) return;

    if (!current.empty() && runDFA(dfa, current, false)) {
        results.push_back(current);
    }

    if ((int)current.size() == maxLen) return;

    for (char c : dfa.alphabet) {
        generateStrings(dfa, current + c, maxLen, results);
    }
}

void printAcceptedStrings(const DFA& dfa, int maxLen) {
    vector<string> results;
    generateStrings(dfa, "", maxLen, results);

    sort(results.begin(), results.end(), [](const string& a, const string& b) {
        if (a.size() != b.size()) return a.size() < b.size();
        return a < b;
    });

    cout << "\n===== 长度 <= " << maxLen << " 的所有合法字符串 =====" << endl;
    if (results.empty()) {
        cout << "无合法字符串。" << endl;
        return;
    }

    for (auto& s : results) {
        cout << s << endl;
    }
}

int main() {
    DFA dfa;
    string filename = "dfa_to.txt";

    if (!loadDFA(filename, dfa)) {
        return 1;
    }

    cout << "DFA 文件读取成功。" << endl;

    if (!validateDFA(dfa)) {
        cout << "DFA 检查失败，请修改输入文件。" << endl;
        return 1;
    }

    cout << "DFA 检查通过。" << endl;

    printDFAInfo(dfa);

    int N;
    cout << "\n请输入最大字符串长度 N: ";
    cin >> N;
    printAcceptedStrings(dfa, N);

    string input;
    cout << "\n请输入要判定的字符串: ";
    cin >> input;
    runDFA(dfa, input, true);

    return 0;
}