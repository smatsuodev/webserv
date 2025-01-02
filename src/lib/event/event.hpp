#ifndef SRC_LIB_EVENT_EVENT_HPP
#define SRC_LIB_EVENT_EVENT_HPP

#include <stdint.h> // NOLINT(*-deprecated-headers): C++98 ではまだ deprecated ではない

class Event {
public:
    // どのイベントを監視するか or どのイベントが起きたか (準備完了したか)
    // TODO: エラーを通知する
    enum EventType {
        kRead = 1,
        kWrite = 1 << 1,
        kError = 1 << 2, // register しなくても発生する. EPOLLERR に相当
    };

    Event();
    /**
     * AutoFd & を受け取りたいが、AutoFd がコピー不可なので、Event もコピー不可になる
     * EventNotifier から Event のコレクションを返すのが困難になってしまう
     * よって、生の fd を管理する。所有権は奪わない。
     */
    explicit Event(int fd, uint32_t typeFlags);
    Event(const Event &);

    Event &operator=(const Event &);

    int getFd() const;
    uint32_t getTypeFlags() const;

private:
    int fd_;
    uint32_t typeFlags_;
};

#endif
