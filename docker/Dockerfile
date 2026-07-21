FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update -o Acquire::Retries=5 \
    && apt-get install -y --no-install-recommends -o Acquire::Retries=5 \
        gcc gcc-doc \
        make make-doc \
        geany \
        valgrind \
        gdb \
        manpages-posix \
        build-essential \
        man-db \
        vim \
        nano \
        python3-pip \
    && pip3 install --break-system-packages \
        gdbgui \
        gevent \
        gevent-websocket \
    && pip3 uninstall --break-system-packages -y eventlet \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

CMD ["/bin/bash"]