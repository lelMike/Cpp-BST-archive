#include <iostream>
#include <stack>
#include <utility>
#include <fstream>
#include <vector>
#include <algorithm>
#include <sstream>

unsigned int calculateFingerprint(const std::string& input) {
    std::string processedString;
    for (char ch : input) {
        if (ch != ' ' && ch != '\t' && ch != '\n') {
            processedString += ch;
        }
    }

    unsigned int fingerprint = 0;
    for (size_t i = 0; i < processedString.length(); i += 4) {
        unsigned int c = 0;
        for (int j = 0; j < 4; ++j) {
            if (i + j < processedString.length()) {
                c |= (static_cast<unsigned int>(processedString[i + j]) << (j * 8));
            }
        }
        fingerprint ^= c;
    }

    return fingerprint;
}

std::string firstFiveLines(const std::string& str) {
    std::istringstream stream(str);
    std::string line;
    std::string firstFive;
    int lineCount = 0;

    while (std::getline(stream, line) && lineCount < 5) {
        firstFive += line + "\n";
        lineCount++;
    }

    return firstFive;
}

class BST {
private:
    struct Node {
        unsigned int key;
        unsigned int id;
        std::string value;
        Node *left, *right;

        Node(unsigned int k, unsigned int i, std::string v) : key(k), id(i), value(v), left(nullptr), right(nullptr) {}
    };

    Node* root;
    Node* existsRoot;
    unsigned int nextID;

    void deleteNode(Node* node) {
        if (!node) return;

        Node *parent = nullptr, *cur = root, *child;
        while (cur && cur->key != node->key) {
            parent = cur;
            cur = (node->key < cur->key) ? cur->left : cur->right;
        }
        if (!cur) return;

        if (cur->left && cur->right) {
            parent = cur;
            Node* succ = cur->right;
            while (succ->left) {
                parent = succ;
                succ = succ->left;
            }
            cur->key = succ->key;
            cur->value = succ->value;
            cur = succ;
        }

        child = (cur->left) ? cur->left : cur->right;
        if (!parent) root = child;
        else if (parent->left == cur) parent->left = child;
        else parent->right = child;

        delete cur;
    }

public:
    BST() : root(nullptr), existsRoot(nullptr), nextID(0) {}

    ~BST() {
        std::stack<Node*> nodes;
        nodes.push(root);
        while (!nodes.empty()) {
            Node* node = nodes.top();
            nodes.pop();
            if (node) {
                nodes.push(node->left);
                nodes.push(node->right);
                delete node;
            }
        }
    }

    void collectNodes(std::vector<Node*>& nodes, Node* cur) {
        if (cur == nullptr) return;
        collectNodes(nodes, cur->left);
        nodes.push_back(cur);
        collectNodes(nodes, cur->right);
    }

    std::vector<Node*> getAllNodes() {
        std::vector<Node*> nodes;
        collectNodes(nodes, root);
        return nodes;
    }

    void writeSortedNodesToFile(const std::string& filename) {
        std::vector<Node*> nodes = getAllNodes();

        // Sort nodes by ID
        std::sort(nodes.begin(), nodes.end(), [](const Node* a, const Node* b) {
            return a->id < b->id;
        });

        // Write sorted nodes to a new file
        std::ofstream outFile(filename);
        for (const auto& node : nodes) {
            outFile << node->value << "\n";
        }
        outFile.close();
    }

    void insertExist(unsigned int key){
        Node **cur = &existsRoot, *newNode = new Node(key, 1, "0");

        while (*cur) {
            Node &node = **cur;
            if (key < node.key) cur = &node.left;
            else cur = &node.right;
        }

        *cur = newNode;
    }

    void insert(std::string value) {
        unsigned int key = calculateFingerprint(value);
        if (search(key, false)) return;

        unsigned int id = nextID++;
        Node **cur = &root, *newNode = new Node(key, id, value);

        while (*cur) {
            Node &node = **cur;
            if (key < node.key) cur = &node.left;
            else if (key > node.key) cur = &node.right;
            else {
                std::cout << "Wystapil konflikt kluczy, ponizej pierwsze 5 linii istniejacego i aktualnego rekordu:\n"
                             "Istniejacy rekord:\n"
                          << firstFiveLines(node.value) << "\n"
                             "Aktualny rekord:\n"
                          << firstFiveLines(value) << "\n"
                             "Wybierz jedna z opcji ponizej, aby rozwiazac konflikt:\n";

                std::cout << "(1) Pomin aktualny rekord,\n"
                             "(2) Pomin aktualny i kazdy kolejny rekord,\n"
                             "(3) Pomin istniejacy i aktualny rekord,\n"
                             "(4) Pomin istniejacy, aktualny i kazdy kolejny rekord,\n"
                             "(5) Zamien dane istniejacego rekordu, na aktualny rekord.\n"
                             "Twoj wybor (1-5): " << std::endl;

                int choice;
                std::cin >> choice;
                while(choice < 1 || choice > 5){
                    std::cout << "Nieprawidłowy wybór, spróbuj ponownie: " << std::endl;
                    std::cin >> choice;
                }

                switch (choice){
                    case 1:
                        return;
                    case 2:
                        insertExist(key);
                        return;
                    case 3:
                        deleteNode(key);
                        return;
                    case 4:
                        deleteNode(key);
                        insertExist(key);
                        return;
                    case 5:
                        node.id = id;
                        node.value = value;
                        return;
                }
            }
        }

        *cur = newNode;
    }

    bool search(unsigned int key, bool whichTree) {
        Node *cur;
        if (whichTree){
            cur = root;
        }
        else{
            cur = existsRoot;
        }

        while (cur) {
            if (key == cur->key) return true;
            else if (key < cur->key) cur = cur->left;
            else cur = cur->right;
        }
        return false;
    }

    void deleteNode(unsigned int key) {
        if (!root) return;

        Node *parent = nullptr, *cur = root;
        while (cur && cur->key != key) {
            parent = cur;
            cur = (key < cur->key) ? cur->left : cur->right;
        }

        if (!cur) return;

        deleteNode(cur);
    }

    int nodeCount() {
        int count = 0;
        std::stack<Node*> stack;
        Node *cur = root;
        while (cur || !stack.empty()) {
            while (cur) {
                stack.push(cur);
                cur = cur->left;
            }
            cur = stack.top(); stack.pop();
            count++;
            cur = cur->right;
        }
        return count;
    }

    int maxHeight() {
        if (!root) return 0;

        int height = 0;
        std::stack<std::pair<Node*, int>> stack;
        stack.push({root, 1});
        while (!stack.empty()) {
            auto [node, curHeight] = stack.top(); stack.pop();
            if (node) {
                height = std::max(height, curHeight);
                stack.push({node->left, curHeight + 1});
                stack.push({node->right, curHeight + 1});
            }
        }
        return height;
    }

    double avgHeight() {
        if (!root) return 0.0;

        int totalHeight = 0, nodeCount = 0;
        std::stack<std::pair<Node*, int>> stack;
        stack.push({root, 1});
        while (!stack.empty()) {
            auto [node, curHeight] = stack.top(); stack.pop();
            if (node) {
                totalHeight += curHeight;
                nodeCount++;
                stack.push({node->left, curHeight + 1});
                stack.push({node->right, curHeight + 1});
            }
        }
        return static_cast<double>(totalHeight) / nodeCount;
    }
};

int main() {
    BST tree;
    std::ifstream file("input.txt");
    if (!file.is_open()) {
        std::cerr << "Blad otwierania pliku" << std::endl;
        return 1;
    }

    int lineCount;
    std::string line, record;
    while (file >> lineCount) {
        std::getline(file, line);
        record.clear();
        for (int i = 0; i < lineCount; ++i) {
            std::getline(file, line);
            record += line + "\n";
        }
        tree.insert(record);
    }

    file.close();

    tree.writeSortedNodesToFile("output.txt");

    return 0;
}