#ifndef SRC_LIB_EVENT_EVENT_HPP
#define SRC_LIB_EVENT_EVENT_HPP

class Event {
public:
    Event();
    /**
     * AutoFd & を受け取りたいが、AutoFd がコピー不可なので、Event もコピー不可になる
     * EventNotifier から Event のコレクションを返すのが困難になってしまう
     * よって、生の fd を管理する。所有権は奪わない。
     */
    explicit Event(int fd);
    Event(const Event &);

    Event &operator=(const Event &);

    int getFd() const;

private:
    int fd_;
};

#endif
