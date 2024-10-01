#include <windows.h>
#include <stdio.h>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <zbar.h>
#include <openssl/evp.h>
#include <openssl/applink.c>
#include "crypto.h"
#include "base64.h"

using namespace std;
using namespace zbar;

BYTE *CaptureScreen(int *width, int *height)
{
    *width = GetSystemMetrics(SM_CXSCREEN);
    *height = GetSystemMetrics(SM_CYSCREEN);
    HDC hScreenDC = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, *width, *height);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);
    BitBlt(hMemoryDC, 0, 0, *width, *height, hScreenDC, 0, 0, SRCCOPY);
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);
    int dataSize = bmp.bmWidthBytes * bmp.bmHeight;
    BYTE *data = (BYTE *)malloc(dataSize);
    BITMAPINFO bmpInfo;
    bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfo.bmiHeader.biWidth = *width;
    bmpInfo.bmiHeader.biHeight = -(*height);
    bmpInfo.bmiHeader.biPlanes = 1;
    bmpInfo.bmiHeader.biBitCount = 24;
    bmpInfo.bmiHeader.biCompression = BI_RGB;
    bmpInfo.bmiHeader.biSizeImage = 0;
    GetDIBits(hMemoryDC, hBitmap, 0, *height, data, &bmpInfo, DIB_RGB_COLORS);
    SelectObject(hMemoryDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);
    return data;
}
void ConvertToGray(unsigned char *rgbData, unsigned char *grayData, int width, int height)
{
    for (int i = 0; i < width * height; ++i)
    {
        unsigned char r = rgbData[3 * i];
        unsigned char g = rgbData[3 * i + 1];
        unsigned char b = rgbData[3 * i + 2];
        unsigned char gray = (unsigned char)(0.299 * r + 0.587 * g + 0.114 * b);
        grayData[i] = gray;
    }
}
void DrawScreenText(const string str, int fontSize)
{
    HDC hdc = GetDC(NULL);
    if (hdc)
    {
        HFONT hFont = CreateFont(fontSize, 0, 0, 0, FW_NORMAL,
                                 FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                 CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH,
                                 "System");
        HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
        SetTextColor(hdc, RGB(0, 255, 0));
        SetBkColor(hdc, RGB(0, 0, 0));
        const char *text = str.c_str();
        TextOut(hdc, 20, 20, text, strlen(text));
        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);
        ReleaseDC(NULL, hdc);
    }
}
unordered_map<string, string> parseUrl(const string url)
{
    unordered_map<string, string> params;
    size_t pos = url.find("?");
    if (pos != string::npos)
    {
        string query = url.substr(pos + 1);
        istringstream stream(query);
        string keyValue;
        while (getline(stream, keyValue, '&'))
        {
            size_t equalPos = keyValue.find('=');
            if (equalPos != string::npos)
            {
                params[keyValue.substr(0, equalPos)] = keyValue.substr(equalPos + 1);
            }
        }
    }
    return params;
}
string md5(const string input)
{
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_len;
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(ctx, EVP_md5(), nullptr);
    EVP_DigestUpdate(ctx, input.c_str(), input.size());
    EVP_DigestFinal_ex(ctx, digest, &digest_len);
    EVP_MD_CTX_free(ctx);
    stringstream ss;
    for (unsigned int i = 0; i < digest_len; i++)
    {
        ss << hex << setw(2) << setfill('0') << (int)digest[i];
    }
    return ss.str();
}
void replaceUrlEncodedChars(string &text)
{
    size_t pos = 0;
    while ((pos = text.find("%3D", pos)) != string::npos)
    {
        text.replace(pos, 3, "=");
        pos += 1;
    }
    pos = 0;
    while ((pos = text.find("%2B", pos)) != string::npos)
    {
        text.replace(pos, 3, "+");
        pos += 1;
    }
    pos = 0;
    while ((pos = text.find("%2F", pos)) != string::npos)
    {
        text.replace(pos, 3, "/");
        pos += 1;
    }
}
void showPassword(string codeData)
{
    printf("got qrcode, decrypting ...\n");
    cryptagram::BlockCipher bc;
    auto urlParams = parseUrl(codeData);
    string ciphertextKey = md5(urlParams["_k"] + urlParams["_d"]);
    printf("aes decrypt key: %s\n", ciphertextKey.c_str());
    string ciphertext = urlParams["_p"];
    replaceUrlEncodedChars(ciphertext);
    printf("aes decrypt: %s\n", ciphertext.c_str());
    string unlockPass;
    bc.Decrypt(base64_decode(ciphertext), ciphertextKey, &unlockPass);
    printf("found password: %s\n", unlockPass.c_str());
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int fontSize = static_cast<int>(screenHeight * 0.2);
    for (int i = 1; i < 1000; i++)
    {
        DrawScreenText(unlockPass + " - " + to_string((1000 - i) / 100) + "s", fontSize);
        Sleep(1);
    }
    printf("done text draw\n");
}
void ScanQRCode(unsigned char *grayData, int width, int height)
{
    ImageScanner scanner;
    Image image(width, height, "Y800", grayData, width * height);
    scanner.scan(image);
    for (Image::SymbolIterator symbol = image.symbol_begin();
         symbol != image.symbol_end(); ++symbol)
    {
        if (symbol->get_type_name() == "QR-Code")
        {
            string codeData = symbol->get_data();
            if (codeData.find("campus.seewo.com/hugo-mobile/#/offlinelock") != string::npos) {
                try {
                    showPassword(codeData);
                } catch (const exception& e) {
                    // Handle the exception here
                    printf("Exception caught! %s",e.what());
                }
            }
        }
    }
}
int main()
{
    int width, height;
    printf("qrcodeScanner by Steve3184. (https://github.com/Steve3184)\n");
    while (true)
    {
        printf("try scan...\n");
        BYTE *screenData = CaptureScreen(&width, &height);
        unsigned char *grayData = (unsigned char *)malloc(width * height);
        ConvertToGray(screenData, grayData, width, height);
        free(screenData);
        ScanQRCode(grayData, width, height);
        free(grayData);
        Sleep(500);
    }
    return 0;
}
