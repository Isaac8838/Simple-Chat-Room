FROM gcc:13.2.0-bookworm

RUN apt install default-libmysqlclient-dev && \
    apt-get update && \
    apt-get install -y gdb && \
    apt-get install -y git

WORKDIR /app

COPY . .

EXPOSE 8888