#include <WString.h>
#include <Arduino.h>

#ifndef MYCRYPTO_H
#define MYCRYPTO_H

class MyCrypto {
  public:
    MyCrypto(const char* key);
    String encrypt(const String& text);
    String decrypt(const String& encryptedText);

  private:
    const char* key;
};

#endif