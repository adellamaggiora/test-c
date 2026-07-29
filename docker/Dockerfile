FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

# Ripristina documentazione e man page rimosse dall'immagine minimale
RUN apt-get update -o Acquire::Retries=5 \
    && yes | unminimize \
    && apt-get install -y --no-install-recommends -o Acquire::Retries=5 \
        build-essential \
        gcc \
        gcc-doc \
        make \
        make-doc \
        gdb \
        valgrind \
        man-db \
        manpages \
        manpages-dev \
        vim \
        nano \
        geany \
        python3-pip \
    && pip3 install --break-system-packages \
        gdbgui \
        gevent \
        gevent-websocket \
    && pip3 uninstall --break-system-packages -y eventlet \
    && mandb \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace

CMD ["/bin/bash"]