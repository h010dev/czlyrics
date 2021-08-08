FROM ubuntu:20.04

RUN apt-get update \
    && apt-get -y upgrade \
    && rm -rf /var/lib/apt/lists/*

# General dependencies
RUN apt-get update \
    && apt-get install -y \
      curl \
      wget \
    && rm -rf /var/lib/apt/lists/*

# GCC and OpenSSL dependencies 
RUN apt-get update \
    && apt-get install -y \
      build-essential \
      checkinstall \
      zlib1g-dev \
      libssl-dev \
    && rm -rf /var/lib/apt/lists/*

# Install OpenSSL from source
RUN cd /usr/local/src/ \
    && wget https://www.openssl.org/source/openssl-1.1.1k.tar.gz \
    && tar -xf openssl-1.1.1k.tar.gz \
    && rm -rf openssl-1.1.1k.targ.gz \
    && cd openssl-1.1.1k \
    && ./config --prefix=/usr/local/ssl --openssldir=/usr/local/ssl shared zlib \
    && make \
    && make test \
    && make install

# Configure OpenSSL installation
RUN cd /etc/ld.so.conf.d/ \
    && touch openssl-1.1.1k.conf \
    && echo "/usr/local/ssl/lib" > openssl-1.1.1k.conf \
    && ldconfig -v \
    && echo "PATH=\"$PATH:/usr/local/ssl/bin\"" > /etc/environment

RUN useradd -ms /bin/bash app 

ADD src/ /src/

Add entrypoint.sh /src/

RUN chown -R app:app /src \
    && chmod +x /src/entrypoint.sh

USER app

WORKDIR /src

EXPOSE 8080/tcp

CMD [ "/src/entrypoint.sh" ]
