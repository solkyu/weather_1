#include <base64.h>
#include <WString.h>
#include "mycrypto.h"

MyCrypto::MyCrypto(const char* key) : key(key) {}

String MyCrypto::encrypt(const String& text) {
  String encryptedText = "";
  size_t textLength = text.length();

  for (size_t i = 0; i < textLength; ++i) {
    char c = text.charAt(i) ^ key[i % strlen(key)];
    encryptedText += c;
  }
  encryptedText = base64::encode(encryptedText);
  return encryptedText;
}

String MyCrypto::decrypt(const String& encryptedText) {
  //encryptedText = base64::decode(encryptedText);
  String decryptedText = "";
  size_t textLength = encryptedText.length();

  for (size_t i = 0; i < textLength; ++i) {
    char c = encryptedText.charAt(i);
    decryptedText += c;
  }
  return decryptedText;
}