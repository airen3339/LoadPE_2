
#include "Encode.h"

std::string Encode::unicode_to_gbk(const std::wstring & wstr)
{
	int nSize = ::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);

    if (0 == nSize)
    {
		throw std::exception("error in conversion.");
    }
    std::vector<char> retString(nSize);
    int ret =::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &retString[0], nSize, NULL, NULL);

    if (ret != nSize)
    {
        throw std::exception("la falla!");
    }

    return std::string(&retString[0]);
}

std::string Encode::unicode_to_utf8(const std::wstring & wstr)
{
	int nSize = ::WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);  

    if (0 == nSize)  
    {  
        throw std::exception("error in conversion.");  
    }  
    std::vector<char> retString(nSize);  
    int ret =::WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &retString[0], nSize, NULL, NULL);  
   
    if (ret != nSize)  
    {  
        throw std::exception("la falla!");  
    }  
   
    return std::string(&retString[0]);  
}

std::wstring Encode::gbk_to_unicode(const std::string & gbk)
{
	int nSize = ::MultiByteToWideChar(CP_ACP, 0, gbk.c_str(), -1, NULL, 0);

	if(0 == nSize)
	{
		throw std::exception("error in conversion.");
	}

	std::vector<wchar_t> retString(nSize);

	int ret = ::MultiByteToWideChar(CP_ACP, 0, gbk.c_str(), - 1, &retString[0], nSize);

	if(ret != nSize)
	{
		throw std::exception("la falla!");
	}

	return std::wstring(&retString[0]);
}

std::string Encode::gbk_to_utf8(const std::string & gbk)
{
	int nLen = MultiByteToWideChar(CP_ACP, 0, gbk.c_str(), -1, NULL, 0);
    WCHAR * wszUTF8 = new WCHAR[nLen];
    MultiByteToWideChar(CP_ACP, 0, gbk.c_str(), -1, wszUTF8, nLen);

    nLen = WideCharToMultiByte(CP_UTF8, 0, wszUTF8, -1, NULL, 0, NULL, NULL);
    char * szUTF8 = new char[nLen];
    WideCharToMultiByte(CP_UTF8, 0, wszUTF8, -1, szUTF8, nLen, NULL, NULL);

    std::string strTemp(szUTF8);
    delete[]wszUTF8;
    delete[]szUTF8;
    return strTemp;
}

std::wstring Encode::utf8_to_unicode(const std::string & utf8)
{
	int nSize = ::MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);  

    if(0 == nSize)
    {  
        throw std::exception("Error in conversion.");
    }  
   
    std::vector<wchar_t> retString(nSize);  
   
    int ret = ::MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &retString[0], nSize);  
   
    if (ret != nSize)  
    {  
        throw std::exception("la falla!");  
    }  
   
    return std::wstring(&retString[0]);  
}

std::string Encode::utf8_to_gbk(const std::string & utf8)
{
    int nLen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
    unsigned short * wszGBK = new unsigned short[nLen + 1];
    memset(wszGBK, 0, nLen * 2 + 2);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, (LPWSTR)wszGBK, nLen);

    nLen = WideCharToMultiByte(CP_ACP, 0, (LPWSTR)wszGBK, -1, NULL, 0, NULL, NULL);
    char *szGBK = new char[nLen + 1];
    memset(szGBK, 0, nLen + 1);
    WideCharToMultiByte(CP_ACP,0, (LPWSTR)wszGBK, -1, szGBK, nLen, NULL, NULL);

    std::string strTemp(szGBK);

	strTemp.size();
    delete[]szGBK;
    delete[]wszGBK;
    return strTemp;
}

LPCSTR Encode::wctoc(LPCWSTR wpBuff)
{
	DWORD dwNum = WideCharToMultiByte(CP_ACP, NULL, wpBuff, -1, NULL, 0, NULL, FALSE);//�ѵ�����������NULL�ĵ����ַ����ĳ��Ȱ�����β��
	char *psText = NULL;
	psText = new char[dwNum];
	if (!psText)
	{
		delete[]psText;
		psText = NULL;
	}
	WideCharToMultiByte(CP_ACP, NULL, wpBuff, -1, psText, dwNum, NULL, FALSE);
	return psText;
}

LPCWSTR Encode::ctowc(LPCSTR pBuff)
{
	DWORD dwNum = MultiByteToWideChar(CP_ACP, 0, pBuff, -1, NULL, 0);//�ѵ�����������NULL�ĵ����ַ����ĳ��Ȱ�����β��

	wchar_t *pwText = NULL;
	pwText = new wchar_t[dwNum];
	if (!pwText)
	{
		delete[]pwText;
		pwText = NULL;
	}
	unsigned nLen = MultiByteToWideChar(CP_ACP, 0, pBuff, -1, pwText, dwNum + 10);
	if (nLen >= 0)
	{
		pwText[nLen] = 0;
	}
	return pwText;
}

std::string Encode::bytes_to_format_string(const char * bytes, int nlen)
{
	int len =0;
	int i = 0;
	//���㻺����Ҫ�Ĵ�С

	len = nlen + 1;//����'\0'�ַ�
	for(i=0; i < nlen; i++)
	{
		//����ת���ַ���Ҫ�� 1
		if(bytes[i] >= 7 && 13 >= bytes[i])
		{
			len++;
		}
		if(bytes[i] == '\0' || bytes[i] == '\\' || bytes[i] == '\'' || bytes[i] == '\"' || bytes[i] == '\?')
		{
			len++;
		}
	}

	char * str = new char[len];
	len = 0;
	//��str���岻Ϊ0������bytes����ת��
	for(i =0; i < nlen; i++)
	{
		switch (*bytes)
		{
		case '\0':
			{
				str[len++] = '\\';
				str[len++] = '0';
			}break;
		case '\a':
			{
				str[len++] = '\\';
				str[len++] = 'a';
			}break;
		case '\b':
			{
				str[len++] = '\\';
				str[len++] = 'b';
			}break;
		case '\t':
			{
				str[len++] = '\\';
				str[len++] = 't';
			}break;
		case '\n':
			{
				str[len++] = '\\';
				str[len++] = 'n';
			}break;
		case '\v':
			{
				str[len++] = '\\';
				str[len++] = 'v';
			}break;
		case '\f':
			{
				str[len++] = '\\';
				str[len++] = 'f';
			}break;
		case '\r':
			{
				str[len++] = '\\';
				str[len++] = 'r';
			}break;
		case '\\':
			{
				str[len++] = '\\';
				str[len++] = '\\';
			}break;
		case '\'':
			{
				str[len++] = '\\';
				str[len++] = '\'';
			}break;
		case '\"':
			{
				str[len++] = '\\';
				str[len++] = '\"';
			}break;
		case '\?':
			{
				str[len++] = '\\';
				str[len++] = '\?';
			}break;
		default:
			{
				str[len++] = *bytes;
			}break;
		}
		bytes++;
	}

	str[len] = '\0';
	std::string format(str);
	delete str;
	return format;
}

std::string Encode::format_string_to_bytes(const char * str, int slen)
{
	int len =0;
	int i=0;
	char * bytes = NULL;

	len = slen;
	for(i=0; i < slen; i++)
	{
		if(str[i] == '\\')
		{
			++i;//ֱ������һλ
			--len;
		}
	}
	bytes = new char[len];

	len =0;
	for(i =0; i < slen ; i++)
	{
		if(str[i] == '\\')
		{
			++i;
			switch (str[i])
			{
			case '0':
				{
					bytes[len++] = '\0';
				}break;
			case 'a':
				{
					bytes[len++] = '\a';
				}break;
			case 'b':
				{
					bytes[len++] = '\b';
				}break;
			case 't':
				{
					bytes[len++] = '\t';
				}break;
			case 'n':
				{
					bytes[len++] = '\n';
				}break;
			case 'v':
				{
					bytes[len++] = '\v';
				}break;
			case 'f':
				{
					bytes[len++] = '\f';
				}break;
			case 'r':
				{
					bytes[len++] = '\r';
				}break;
			case '\\':
				{
					bytes[len++] = '\\';
				}break;
			/*case '\'':
				{
					bytes[len++] = '\'';
				}break;
			case '\"':
				{
					bytes[len++] = '\"';
				}break;
			case '\?':
				{
					bytes[len++] = '\?';
				}break;*/
			default:
				{
					bytes[len++] = str[i];
				}break;
			}
		}
		else
		{
			bytes[len++] = str[i];
		}
	}

	std::string format(bytes, len);
	delete bytes;
	return format;
}

std::string Encode::base64_encode(const unsigned char * str,int bytes)
{
    int num = 0,bin = 0;
    std::string encode_result;
    const unsigned char * current;
    current = str;

	/*����Base64����ʹ�õı�׼�ֵ�*/
	char EncodeTable[] = { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" };
    while(bytes > 2) {
        encode_result += EncodeTable[current[0] >> 2];
        encode_result += EncodeTable[((current[0] & 0x03) << 4) + (current[1] >> 4)];
        encode_result += EncodeTable[((current[1] & 0x0f) << 2) + (current[2] >> 6)];
        encode_result += EncodeTable[current[2] & 0x3f];

        current += 3;
        bytes -= 3;
    }
    if(bytes > 0)
    {
        encode_result += EncodeTable[current[0] >> 2];
        if(bytes%3 == 1) {
            encode_result += EncodeTable[(current[0] & 0x03) << 4];
            encode_result += "==";
        } else if(bytes%3 == 2) {
            encode_result += EncodeTable[((current[0] & 0x03) << 4) + (current[1] >> 4)];
            encode_result += EncodeTable[(current[1] & 0x0f) << 2];
            encode_result += "=";
        }
    }
    return encode_result;
}

std::string Encode::base64_decode(const char *str,int length)
{
	//������ֵ�
    const char DecodeTable[] =
    {
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -1, -1, -2, -2, -1, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -1, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, 62, -2, -2, -2, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -2, -2, -2, -2, -2, -2,
        -2,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -2, -2, -2, -2, -2,
        -2, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2
    };
    int bin = 0,i=0,pos=0;
    std::string decode_result;
    const char *current = str;
    char ch;
    while( (ch = *current++) != '\0' && length-- > 0 )
    {
        if (ch == '=') 
		{ // ��ǰһ���ַ��ǡ�=����
            /*
            ��˵��һ������ڽ���ʱ��4���ַ�Ϊһ�����һ���ַ�ƥ�䡣
            ����������
                1�����ĳһ��ƥ��ĵڶ����ǡ�=���ҵ������ַ����ǡ�=����˵������������ַ������Ϸ���ֱ�ӷ��ؿ�
                2�������ǰ��=�����ǵڶ����ַ����Һ�����ַ�ֻ�����հ׷�����˵�������������Ϸ������Լ�����
            */
            if (*current != '=' && (i % 4) == 1) 
			{
                return NULL;
            }
            continue;
        }
        ch = DecodeTable[ch];
        //�������Ҫ�������������в��Ϸ����ַ�
        if (ch < 0 ) 
		{ /* a space or some other separator character, we simply skip over */
            continue;
        }
        switch(i % 4)
        {
            case 0:
                bin = ch << 2;
                break;
            case 1:
                bin |= ch >> 4;
                decode_result += bin;
                bin = ( ch & 0x0f ) << 4;
                break;
            case 2:
                bin |= ch >> 2;
                decode_result += bin;
                bin = ( ch & 0x03 ) << 6;
                break;
            case 3:
                bin |= ch;
                decode_result += bin;
                break;
        }
        i++;
    }
    return decode_result;
}