#pragma once
#include <unordered_set>

class KeyManager {
private:
    std::unordered_set<int> downKeys;        // 일반 키 (unsigned char을 int로 저장)
    std::unordered_set<int> downSpecial;     // GLUT special keys (GLUT_KEY_*)

public:
    KeyManager() = default;

    // 일반키
    void pressKey(int key) { downKeys.insert(key); }
    void releaseKey(int key) { downKeys.erase(key); }
    bool isKeyDown(int key) const { return downKeys.find(key) != downKeys.end(); }

    // 스페셜 키
    void pressSpecial(int key) { downSpecial.insert(key); }
    void releaseSpecial(int key) { downSpecial.erase(key); }
    bool isSpecialDown(int key) const { return downSpecial.find(key) != downSpecial.end(); }

    void clearAll() { downKeys.clear(); downSpecial.clear(); }
};

