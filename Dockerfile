FROM alpine:3 AS builder

RUN apk update && apk add --no-cache \
    g++ \
    cmake \
    make \
    git

WORKDIR /src

COPY CMakeLists.txt .
COPY core/ ./core/
COPY infra/ ./infra/
COPY app/ ./app/
COPY cli/ ./cli/
COPY tests/ ./tests/

RUN cmake -B build -DCMAKE_BUILD_TYPE=Release

RUN cmake --build build

RUN cd build && ctest --output-on-failure

# ==========================================

FROM alpine:3

RUN apk add --no-cache libstdc++

WORKDIR /app

COPY --from=builder /src/build/cli/cobblestone /usr/local/bin/cobblestone

RUN mkdir -p /app/notes

ENTRYPOINT ["cobblestone"]
