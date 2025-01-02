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
        kRecoverable,

        // IO
        // ReSharper disable CppInconsistentNaming
        kIOUnknown, // EAGAIN も含む
        // ReSharper restore CppInconsistentNaming

        // Validation
        kValidationUnknown,
        kValidationInvalidArgs,

        // Parse
        // 出し分けてもあんまり意味はないので、分類してない
        kParseUnknown,
    };

    // mapErr で使う想定
    AppError recover(AppError err);
    // ReSharper disable once CppInconsistentNaming
    AppError recoverIOError(AppError err);
}

#endif
