FROM debian:12-slim

RUN apt-get update && \
    apt-get install -y \
    cmake build-essential libssl-dev zlib1g-dev libpcre3-dev \
    pkg-config libc-ares-dev libre2-dev \
    build-essential gdb \
    libgtest-dev libbenchmark-dev libcurl4-openssl-dev google-mock libgmock-dev \
    git wget curl unzip

RUN wget https://github.com/nginx/nginx/archive/refs/tags/release-1.25.1.zip && \
    unzip /release-1.25.1.zip && \ 
    cd /nginx-release-1.25.1 && \
    auto/configure --with-compat

RUN git clone https://github.com/alfianabdi/nginx-otel.git && \
    cd nginx-otel && \
    mkdir build && \
    cd build && \
    cmake -DNGX_OTEL_NGINX_BUILD_DIR=/nginx-release-1.25.1/objs .. && \
    make

FROM scratch
COPY --from=0 /nginx-otel/build/ngx_otel_module.so /ngx_otel_module.so
