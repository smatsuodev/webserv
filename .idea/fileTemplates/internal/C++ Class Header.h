#[[#ifndef]]# ${DIR_PATH.toUpperCase().replace("/", "_")}_${INCLUDE_GUARD}
#[[#define]]# ${DIR_PATH.toUpperCase().replace("/", "_")}_${INCLUDE_GUARD}

${NAMESPACES_OPEN}

class ${NAME} {
public:
    ${NAME}();
    ~${NAME}();
private:
};

${NAMESPACES_CLOSE}

#[[#endif]]#
