#ifndef SRC_LIB_UTILS_TYPES_ERROR_HPP
#define SRC_LIB_UTILS_TYPES_ERROR_HPP

namespace error {
    /**
     * TODO: もっといい感じに設計したい
     * 型から起こり得るエラーがわからないので、caller のエラーハンドリングが難しくなる
     * また、エラー固有のメタ情報を一緒に返すことができない
     * std::pair<AppError, std::string> とすればメッセージは付与できるが、イケてない
     */
    // 必要に応じて種類を追加する
    enum AppError {
        kUnknown,

        // IO
        // ReSharper disable CppInconsistentNaming
        kIOUnknown,
        kIOWouldBlock,
        // ReSharper restore CppInconsistentNaming

        // Validation
        kValidationUnknown,
        kValidationInvalidArgs,
    };
}

#endif
