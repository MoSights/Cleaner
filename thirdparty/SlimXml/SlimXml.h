#ifndef __SLIM_XML_H__
#define __SLIM_XML_H__

#include <string>
#include <list>
#include <istream>

#pragma warning(push)
#pragma warning(disable:4251)

//disable this to make parsing a little bit faster
//#define SLIM_TRANSFER_CHARACTER

#if defined(SLIMLIB_EXPORTS)
#if defined(_MSC_VER)
//#define SLIMLIB_API __declspec(dllexport)
#define SLIMLIB_API
#else
#define SLIMLIB_API 
#endif
#else
#if defined(_MSC_VER)
//#define SLIMLIB_API __declspec(dllimport)
#define SLIMLIB_API
#else
#define SLIMLIB_API 
#endif
#endif

namespace slim
{
#undef SLIM_USE_WCHAR
#if defined (_MSC_VER) && defined (UNICODE)
	#define SLIM_USE_WCHAR
#endif

enum Encode
{
	ANSI = 0,
	UTF_8,
	UTF_8_NO_MARK,
	UTF_16,
	UTF_16_BIG_ENDIAN,

#if defined (SLIM_USE_WCHAR) || defined (__GNUC__)
	DefaultEncode = UTF_8
#else
	DefaultEncode = ANSI
#endif
};

#ifdef SLIM_USE_WCHAR
	typedef wchar_t Char;
	#define T(str) L##str
	#define StrToI _wtoi
	#define StrToF _wtof
	#define Sprintf swprintf
	#define Sscanf swscanf
	#define Strlen wcslen
	#define Strcmp wcscmp
	#define Stricmp _wcsicmp
	#define Strncmp wcsncmp
	#define Memchr wmemchr
	#define Strcpy wcscpy
#else
	typedef char Char;
	#define T(str) str
	#define StrToI atoi
	#define StrToF atof
#if defined (__GNUC__)
	#define Sprintf snprintf
#elif defined (_MSC_VER)
	#define Sprintf sprintf_s
#endif
	#define Sscanf sscanf
	#define Strlen strlen
	#define Strcmp strcmp
	#define Stricmp _stricmp
	#define Strncmp strncmp
	#define Memchr memchr
	#define Strcpy strcpy
#endif

class XmlAttribute;
class XmlNode;

typedef std::basic_string<Char> String;
typedef std::list<XmlAttribute*> AttributeList;
typedef std::list<XmlNode*> NodeList;

typedef SLIMLIB_API AttributeList::const_iterator AttributeIterator;
typedef SLIMLIB_API NodeList::const_iterator NodeIterator;

enum NodeType
{
	DOCUMENT = 0,
	ELEMENT,
	COMMENT,
	DECLARATION
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class SLIMLIB_API XmlBase
{
	friend class XmlDocument;

public:
	XmlBase();
	~XmlBase();

	const Char* getName() const;
	void setName(const Char* name);

	const Char*	getString() const;
	bool getBool() const;
	int	getInt() const;
	unsigned long getHex() const;
	float getFloat() const;
	double getDouble() const;

	void setString(const Char* value);
	void setString(const String& value);
	void setBool(bool value);
	void setInt(int value);
	void setHex(unsigned long value);
	void setFloat(float value);
	void setDouble(double value);

private:
	void assignString(Char* &str, Char* value, size_t length, bool transferCharacter);

protected:
	Char* m_name;
	Char* m_value;
	bool m_nameAllocated;
	bool m_valueAllocated;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class SLIMLIB_API XmlAttribute : public XmlBase
{
public:
	XmlAttribute();
	~XmlAttribute();
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class SLIMLIB_API XmlNode : public XmlBase
{
public:
	XmlNode(NodeType type, XmlNode* parent);
	~XmlNode();

public:
	NodeType getType() const;

	bool isEmpty() const;

	XmlNode* getParent() const;

	bool hasChild() const;

	XmlNode* getFirstChild(NodeIterator& iter) const;
	XmlNode* getNextChild(NodeIterator& iter) const;
	XmlNode* getChild(NodeIterator iter) const;
	size_t getChildCount() const;

	XmlNode* findChild(int index) const;
	XmlNode* findChild(const Char* name) const;
	XmlNode* findFirstChild(const Char* name, NodeIterator& iter) const;
	XmlNode* findNextChild(const Char* name, NodeIterator& iter) const;
	size_t getChildCount(const Char* name) const;

	void removeChild(XmlNode* node);
	void clearChild();
	XmlNode* addChild(const Char* name = NULL, NodeType = ELEMENT);
	XmlNode* addChild(XmlNode* before_node, const Char* child_name, NodeType type = ELEMENT);
	bool hasAttribute() const;
	XmlAttribute* findAttribute(const Char* name) const;

	const Char* readAttributeAsString(const Char* name, const Char* defaultValue = T("")) const;
	bool readAttributeAsBool(const Char* name, bool defaultValue = false) const;
	int readAttributeAsInt(const Char* name, int defaultValue = 0) const;
	void readAttributeAsIntArray(const Char* name, int* out, unsigned long length, int defaultValue = 0) const;
	unsigned long readAttributeAsHex(const Char* name, unsigned long defaultValue = 0) const;
	float readAttributeAsFloat(const Char* name, float defaultValue = 0.0f) const;
	double readAttributeAsDouble(const Char* name, double defaultValue = 0.0) const;
	unsigned long readAttributeAsEnum(const Char* name, const Char* const* enumNames, 
		unsigned long enumCount, unsigned long defaultValue = 0) const;

	XmlAttribute* getFirstAttribute(AttributeIterator& iter) const;
	XmlAttribute* getNextAttribute(AttributeIterator& iter) const;

	void removeAttribute(XmlAttribute* attribute);
	void clearAttribute();

	XmlAttribute* addAttribute(const Char* name = NULL, const Char* value = NULL);
	XmlAttribute* addAttribute(const Char* name, bool value);
	XmlAttribute* addAttribute(const Char* name, int value);
	XmlAttribute* addAttribute(const Char* name, float value);
	XmlAttribute* addAttribute(const Char* name, double value);

protected:
	void writeNode(String& output, int depth) const;
	void writeChildNodes(String& output, int depth) const;
	void writeTransferredString(String& output, const Char* input) const;

private:
	NodeType m_type;
	AttributeList m_attributes;
	XmlNode* m_parent;
	NodeList m_children;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class SLIMLIB_API XmlDocument : public XmlNode
{
public:
	XmlDocument();
	~XmlDocument();

	bool loadFromFile(const Char* filename);
	bool loadFromStream(std::istream& input);
	bool loadFromMemory(const char* buffer, size_t size);

	bool save(const Char* filename, Encode encode = DefaultEncode) const;

private:
	bool reallyLoadFromMemory(char* buffer, size_t size, bool copiedMemory);

	bool parse(Char* input, size_t size);
	bool findLabel(Char* &begin, size_t size, Char* &label, size_t &labelSize);
	bool parseLabel(XmlNode* node, Char* label, size_t labelSize);

private:
	char* m_buffer;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

size_t utf8toutf16(const char* u8, size_t size, wchar_t* u16, size_t outBufferSize);
size_t utf16toutf8(const wchar_t* u16, size_t size, char* u8, size_t outBufferSize);
Encode detectEncode(const char* str, size_t size, bool& multiBytes);
}

#pragma warning(pop)

#endif
