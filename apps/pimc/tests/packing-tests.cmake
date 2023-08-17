
file(READ ${SOURCE_YAML} YAML_CONTENTS)

string(PREPEND
        YAML_CONTENTS
        [=[
namespace {
char const* pvConfigs = R"yaml(
]=])

string(APPEND
        YAML_CONTENTS
        [=[
)yaml"\;
} // anon.namespace
]=])

file(WRITE ${INCLUDE_FILE} ${YAML_CONTENTS})
