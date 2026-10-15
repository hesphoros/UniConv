if(NOT DEFINED HEADER)
    message(FATAL_ERROR "HEADER is required")
endif()

file(READ "${HEADER}" HEADER_CONTENT)

set(FORBIDDEN_PATTERNS
    "execution_character_set"
    "UNICONV_CONFIG_LEGACY_AUTO_EXECUTION_CHARSET"
    "UNICONV_U8_LITERAL"
    "UNICONV_UTF8_VIEW"
    "UNICONV_CP_[A-Za-z0-9_]+"
    "__UNICONV_H__"
    "warning\\([^\\n]*4819"
    "(^|[^A-Za-z0-9_])CPP_STANDARD([^A-Za-z0-9_]|$)"
)

foreach(FORBIDDEN_PATTERN IN LISTS FORBIDDEN_PATTERNS)
    if(HEADER_CONTENT MATCHES "${FORBIDDEN_PATTERN}")
        message(FATAL_ERROR
            "Forbidden UniConv 4.0 macro surface found in ${HEADER}: ${FORBIDDEN_PATTERN}"
        )
    endif()
endforeach()
