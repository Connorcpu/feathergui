// Copyright �2017 Black Sphere Studios
// For conditions of distribution and use, see copyright notice in "bss_util.h"

#ifndef __C_XML_H__BSS__
#define __C_XML_H__BSS__

#include "cDynArray.h"
#include "cHash.h"
#include "LLBase.h"
#include "cStr.h"

namespace bss_util {
  // Represents an XML value converted to various forms.
  struct BSS_COMPILER_DLLEXPORT cXMLValue
  {
    cXMLValue() : Float(0.0), Integer(0) {}
    cStr Name;
    cStr String;
    double Float;
    int64_t Integer;

    BSS_FORCEINLINE operator bool() const { return Integer!=0; } // If the string is "true" the integer gets set to 1 by the parser.
    BSS_FORCEINLINE operator char() const { return (char)Integer; }
    BSS_FORCEINLINE operator short() const { return (short)Integer; }
    BSS_FORCEINLINE operator int() const { return (int)Integer; }
    BSS_FORCEINLINE operator long() const { return (long)Integer; }
    BSS_FORCEINLINE operator int64_t() const { return (int64_t)Integer; }
    BSS_FORCEINLINE operator uint8_t() const { return (uint8_t)Integer; }
    BSS_FORCEINLINE operator uint16_t() const { return (uint16_t)Integer; }
    BSS_FORCEINLINE operator uint32_t() const { return (uint32_t)Integer; }
    BSS_FORCEINLINE operator unsigned long() const { return (unsigned long)Integer; }
    BSS_FORCEINLINE operator uint64_t() const { return (uint64_t)Integer; }
    BSS_FORCEINLINE operator float() const { return (float)Float; }
    BSS_FORCEINLINE operator double() const { return Float; }
    BSS_FORCEINLINE operator const char*() const { return String; }
  };

  // Simple cXMLNode. Note that return values from GetAttribute are invalid after adding a node or an attribute.
  struct BSS_DLLEXPORT cXMLNode : LLBase<cXMLNode>
  {
    cXMLNode(const cXMLNode& copy);
    cXMLNode(cXMLNode&& mov);
    explicit cXMLNode(const char* parse=0);
    explicit cXMLNode(std::istream& stream);
    BSS_FORCEINLINE const char* GetName() const { return _name; }
    BSS_FORCEINLINE const cXMLNode* GetNode(size_t index) const { return index>=_nodes.Length()?0:_nodes[index].get(); }
    BSS_FORCEINLINE const cXMLNode* GetNode(const char* name) const { return GetNode(_nodehash[name]); }
    BSS_FORCEINLINE size_t GetNodes() const { return _nodes.Length(); }
    BSS_FORCEINLINE const cXMLValue* GetAttribute(size_t index) const { return index>=_attributes.Length()?0:(_attributes+index); }
    BSS_FORCEINLINE const cXMLValue* GetAttribute(const char* name) const { return GetAttribute(_attrhash[name]); }
    BSS_FORCEINLINE const char* GetAttributeString(const char* name) const { const cXMLValue* r = GetAttribute(_attrhash[name]); return !r ? 0 : r->String.c_str(); }
    BSS_FORCEINLINE const int64_t GetAttributeInt(const char* name) const { const cXMLValue* r = GetAttribute(_attrhash[name]); return !r ? 0 : r->Integer; }
    BSS_FORCEINLINE const double GetAttributeFloat(const char* name) const { const cXMLValue* r = GetAttribute(_attrhash[name]); return !r ? 0 : r->Float; }
    BSS_FORCEINLINE size_t GetAttributes() const { return _attributes.Length(); }
    BSS_FORCEINLINE const cXMLValue& GetValue() const { return _value; }
    BSS_FORCEINLINE void SetName(const char* name) { _name = name; }
    cXMLNode* BSS_FASTCALL AddNode(const cXMLNode& node);
    cXMLNode* BSS_FASTCALL AddNode(const char* name);
    cXMLValue* BSS_FASTCALL AddAttribute(const cXMLValue& value);
    cXMLValue* BSS_FASTCALL AddAttribute(const char* name);
    bool BSS_FASTCALL RemoveNode(size_t index);
    bool BSS_FASTCALL RemoveNode(const char* name);
    bool BSS_FASTCALL RemoveAttribute(size_t index);
    bool BSS_FASTCALL RemoveAttribute(const char* name);
    void BSS_FASTCALL SetValue(double value);
    void BSS_FASTCALL SetValue(int64_t value);
    void BSS_FASTCALL SetValue(const char* value);
    BSS_FORCEINLINE const std::unique_ptr<cXMLNode>* begin() const noexcept { return _nodes.begin(); }
    BSS_FORCEINLINE const std::unique_ptr<cXMLNode>* end() const noexcept { return _nodes.end(); }

    cXMLNode& operator=(const cXMLNode& copy);
    cXMLNode& operator=(cXMLNode&& mov);
    BSS_FORCEINLINE const cXMLNode* operator[](size_t index) const { return GetNode(index); }
    BSS_FORCEINLINE const cXMLNode* operator[](const char* name) const { return GetNode(name); }
    BSS_FORCEINLINE const cXMLValue* operator()(size_t index) const { return GetAttribute(index); }
    BSS_FORCEINLINE const cXMLValue* operator()(const char* name) const { return GetAttribute(name); }

  protected:
    cXMLNode* BSS_FASTCALL _addnode(std::unique_ptr<cXMLNode> && n);
    cXMLValue* BSS_FASTCALL _addattribute(cXMLValue && v);
    static bool BSS_FASTCALL _match(std::istream& stream, cStr& out, const char* pattern, bool reset = false);
    bool BSS_FASTCALL _parse(std::istream& stream, cStr& buf);
    void BSS_FASTCALL _parseinner(std::istream& stream, cStr& buf);
    void BSS_FASTCALL _parseattribute(cStr& buf);
    static void BSS_FASTCALL _parseentity(std::istream& stream, cStr& target);
    static void BSS_FASTCALL _evalvalue(cXMLValue& val);
    void BSS_FASTCALL _writeattribute(std::ostream& stream) const;
    static void BSS_FASTCALL _writestring(std::ostream& stream, const char* s);
    void BSS_FASTCALL _write(std::ostream& stream, bool pretty, int depth) const;

    friend class cXML;

    cDynArray<std::unique_ptr<cXMLNode>, size_t, CARRAY_MOVE> _nodes;
    cHash<cStr, size_t, false, CARRAY_SAFE> _nodehash;
    cDynArray<cXMLValue, size_t, CARRAY_MOVE> _attributes;
    cHash<cStr, size_t, false, CARRAY_SAFE> _attrhash;
    cXMLValue _value;
    cStr _name;
  };

  // Tiny XML parser
  class BSS_DLLEXPORT cXML : public cXMLNode
  {
  public:
    cXML(const cXML& copy);
    cXML(cXML&& mov);
    explicit cXML(const char* source=0);
    explicit cXML(std::istream& stream);
    void BSS_FASTCALL Write(const char* file, bool pretty=true) const;
    void BSS_FASTCALL Write(std::ostream& stream, bool pretty=true) const;

    inline cXML& operator=(const cXML& copy) { cXMLNode::operator=(copy); return *this; }
    inline cXML& operator=(cXML&& mov) { cXMLNode::operator=(mov); return *this; }
    BSS_FORCEINLINE const cXMLNode* operator[](size_t index) const { return GetNode(index); }
    BSS_FORCEINLINE const cXMLNode* operator[](const char* name) const { return GetNode(name); }
    BSS_FORCEINLINE const cXMLValue* operator()(size_t index) const { return GetAttribute(index); }
    BSS_FORCEINLINE const cXMLValue* operator()(const char* name) const { return GetAttribute(name); }

  protected:
    void _initialparse(std::istream& stream, cStr& buf);
  };
}



#endif