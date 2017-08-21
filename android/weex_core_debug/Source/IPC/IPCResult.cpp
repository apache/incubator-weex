#include "IPCResult.h"

namespace {
class VoidResult : public IPCResult {
public:
    const void* getData() override;
    IPCType getType() override;
    const uint16_t* getStringContent() override;
    size_t getStringLength() override;
    const char* getByteArrayContent() override;
    size_t getByteArrayLength() override;
};

template <typename T>
struct TypeTrait {
};

template <>
struct TypeTrait<int32_t> {
    static const IPCType s_type = IPCType::INT32;
};

template <>
struct TypeTrait<double> {
    static const IPCType s_type = IPCType::DOUBLE;
};

template <typename T>
class ValueResult : public IPCResult {
public:
    explicit ValueResult(T val);
    const void* getData() override;
    IPCType getType() override;
    const uint16_t* getStringContent() override;
    size_t getStringLength() override;
    const char* getByteArrayContent() override;
    size_t getByteArrayLength() override;

private:
    typedef T value_type;
    typedef TypeTrait<value_type> trait_type;

    value_type m_value;
    static const IPCType s_type = trait_type::s_type;
};

class StringResult : public IPCResult {
public:
    StringResult(JNIEnv* env, jstring val);
    ~StringResult();

    const void* getData() override;
    IPCType getType() override;
    const uint16_t* getStringContent() override;
    size_t getStringLength() override;
    const char* getByteArrayContent() override;
    size_t getByteArrayLength() override;
    void setJSON();

private:
    JNIEnv* m_env;
    jstring m_value;
    const jchar* m_cvalue{ nullptr };
    size_t m_length{ 0U };
    bool m_isJSON{ false };
};

const void*
VoidResult::getData()
{
    return nullptr;
}

IPCType VoidResult::getType()
{
    return IPCType::VOID;
}

const uint16_t* VoidResult::getStringContent()
{
    return nullptr;
}

size_t VoidResult::getStringLength()
{
    return 0U;
}

const char* VoidResult::getByteArrayContent()
{
    return nullptr;
}

size_t VoidResult::getByteArrayLength()
{
    return 0U;
}

template <typename T>
ValueResult<T>::ValueResult(T val)
    : m_value(val)
{
}

template <typename T>
const void* ValueResult<T>::getData()
{
    return &m_value;
}

template <typename T>
IPCType ValueResult<T>::getType()
{
    return s_type;
}

template <typename T>
const uint16_t* ValueResult<T>::getStringContent()
{
    return nullptr;
}

template <typename T>
size_t ValueResult<T>::getStringLength()
{
    return 0U;
}

template <typename T>
const char* ValueResult<T>::getByteArrayContent()
{
    return nullptr;
}

template <typename T>
size_t ValueResult<T>::getByteArrayLength()
{
    return 0U;
}

StringResult::StringResult(JNIEnv* env, jstring val)
    : m_env(env)
    , m_value(static_cast<jstring>(env->NewLocalRef(val)))
{
    m_cvalue = env->GetStringChars(m_value, nullptr);
    m_length = env->GetStringLength(m_value);
}

StringResult::~StringResult()
{
    if (m_cvalue)
        m_env->ReleaseStringChars(m_value, m_cvalue);
    m_env->DeleteLocalRef(m_value);
}

const void* StringResult::getData()
{
    return nullptr;
}

IPCType StringResult::getType()
{
    return m_isJSON ? IPCType::JSONSTRING : IPCType::STRING;
}

const uint16_t* StringResult::getStringContent()
{
    return m_cvalue;
}

size_t StringResult::getStringLength()
{
    return m_length;
}

const char* StringResult::getByteArrayContent()
{
    return nullptr;
}

size_t StringResult::getByteArrayLength()
{
    return 0U;
}

void StringResult::setJSON()
{
    m_isJSON = true;
}
}

std::unique_ptr<IPCResult> createVoidResult()
{
    return std::unique_ptr<IPCResult>(new VoidResult);
}

std::unique_ptr<IPCResult> createInt32Result(int32_t val)
{
    return std::unique_ptr<IPCResult>(new ValueResult<int32_t>(val));
}

std::unique_ptr<IPCResult> createDoubleResult(double val)
{
    return std::unique_ptr<IPCResult>(new ValueResult<double>(val));
}

std::unique_ptr<IPCResult> createStringResult(JNIEnv* env, jstring str)
{
    return std::unique_ptr<IPCResult>(new StringResult(env, str));
}

std::unique_ptr<IPCResult> createJSONStringResult(JNIEnv* env, jstring str)
{
    std::unique_ptr<StringResult> result(new StringResult(env, str));
    result->setJSON();
    return std::unique_ptr<IPCResult>(result.release());
}
