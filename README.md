# 프록시랩 접근 순서 및 방향

## Part1 : Implementing a sequential web proxy

> ❗️**sequential web proxy란?**
한 번에 하나씩 요청을 처리하는 웹 서버이다.
트랜잭션이 완료되면 다음 커넥션을 처리한다. 처리 도중에 모든 다른 커넥션이 무시되므로 성능 문제가 있다.
(트랜잭션: 더 이상 쪼갤 수 없는 작업의 단위)

기초 sequential proxy를 구현하기 위한 첫 번째 단계는 HTTP/1.0 GET request를 처리하는 것이다.
POST같은 다른 request 타입은 일단 보류하자.


프록시는 커맨드 라인에 명시되어 있는 포트 번호로 들어오는 요청을 받을 준비를 해야 한다. 즉, 듣고 있어야 한다.
클라이언트와 프록시 사이의 커넥션이 만들어지면, 프록시는 클라이언트가 보낸 request 전체를 읽고 파싱해야 한다. 이때, 프록시는 클라이언트가 보낸 HTTP request가 유효한지 검사해야 한다. 그리고 그 request가 유효하다면, 프록시는 웹 서버와 커넥션을 형성하고 클라이어트가 요청했던 객체를 서버에게 요청한다. 그리고 마지막으로 프록시는 서버의 응답을 읽고 그 응답을 클라이언트에게 보낸다.


### 1.1 HTTP/1.0 GET request
유저가 웹 브라우저의 주소창에 http://www.cmu.edu/hub/index.html 라는 URL을 입력하면, 브라우저는 프록시에게 아래와 같은 한 줄의 HTTP request를 보낸다.

> 👉 GET http://www.cmu.edu/hub/index.html HTTP/1.1

이 경우에서, 프록시는 적어도 다음과 같은 형태로 브라우저가 보낸 request를 파싱해야 한다.
1) hostname - www.cmu.edu
2) path or query 등등 - /hub/index.html
그럼 프록시는 www.cmu.edu(에 대응되는 서버)와 커넥션을 형성하고 아래와 같은 HTTP request를 보낸다.
> 👉GET /hub/index.html HTTP/1.0

_(프록시가 이미 서버와 커넥션을 만들었기 때문에 그 서버에게 요청할 데이터만 보내면 되는 구조이다.)_


HTTP request의 한 줄 한 줄은 carriage return, ‘\r\n’으로 끝나야 한다.
또한 HTTP request가 완전히 끝났음을 알려주기 위해 ‘\r\n’을 적어야 한다.
위의 경우에서 웹 브라우저는 HTTP/1.1로 요청을 보내는 반면, 프록시는 HTTP/1.0으로 요청을 보내는 것을 알 수 있다. 모던 웹 브라우저는 HTTP/1.1 으로 요청을 보내지만, 프록시는 요청들을 처리한 후(아마 유효성 검사하고 서버와 커넥션을 형성하는 것) HTTP/1.0 으로 요청을 보낸다.
HTTP request는 굉장히 복잡하다. 따라서 공부해야 한다.


### 1.2 Request headers
Host, User-Agent, Connection, Proxy-Connection 헤더에 대해 알아보자.
#### 1. Host
Host  헤더는 항상 헤더에 포함되어야 한다. HTTP/1.0 버전에서는 Host 헤더를 포함하는 것이 필수는 아니지만, 가상 호스팅을 사용하는 웹 서버로부터 응답을 받기 위해서는 필요하다.

> 👆 Virtual Hosting이란?
웹 호스팅 업자들이 서버 한 대를 여러 고객이 공유하도록 해서 저렴한 웹 호스팅 서비스를 제공하고, 이 기능을 공유 호스팅/ 가상 호스팅이라고 한다.
가상 호스팅에는 4가지 종류가 있고, 그 중에 Host 헤더를 통한 가상 호스팅이 있다.
**→ Host 헤더를 통한 가상 호스팅**
같은 IP를 쓰면서도 하나의 물리 서버에서 여러 가상 호스트를 구분할 수 있어야 한다. 이를 위해서 HTTP/1.1 프로토콜에서는 HTTP 요청을 보낼 때 Host 헤더를 전달해야 한다.
**→ Host 헤더를 사용할 때 규칙(아래 링크 참고)
→ Host 헤더의 누락(아래 링크 참고)
→ Host 헤더 해석(아래 링크 참고)**


(https://eminentstar.tistory.com/46)
Host 헤더는 엔드 서버의 hostname을 의미한다. 예를 들어 http://www.cmu.edu/hub/index.html 에 접근하기 위해서 프록시는 아래와 같이 Host header를 추가해야 한다.

> 👉 Host: www.cmu.edu


웹 브라우저가 HTTP request에 Host header를 추가하면 된다. 이렇게 Host 헤더가 추가되어 있는 경우에 프록시는 Host header를 사용하면 된다.


#### 2. User-Agent 
아래와 같이 User-Agent 헤더를 추가하자.
> 👉 User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3)
Gecko/20120305 Firefox/10.0.3


User-Agent 헤더는 두 개의 라인으로 제공된다. 왜냐하면 한 줄에 다 담기에는 길기 때문이다. 하지만 프록시는 이 헤더를 한 줄로 보내야 한다.
User-Agent  헤더는 운영체제와 브라우저같은 파라미터의 관점에서 클라이언트를 식별할 수 있다. 그리고 웹 서버는 종종 그들이 제공하는 콘텐츠를 조작하기 위해서 이 식별 정보를 이용하기도 한다.

> 👆 User-Agent란?
사용자를 대표하는 컴퓨터 프로그램으로, 웹 맥락에선 브라우저를 의미한다.
브라우저는 서버에 보내는 모든 요청에 사용자 에이전트 문자열이라고 불리는, 자신의 정체를 알리는 User-Agent 헤더를 보낸다.
이 문자열은 보통 브라우저 종류, 버전 번호, 호스트 운영체제를 포함한다.
참고로 User-Agent Spoofing이란, 자신의 정체를 숨기고 다른 클라이언트인 척 하기 위해 가짜 사용자 에이전트 문자열을 보내는 것이다.
js의 navigator.userAgent 속성을 통해서 사용자 에이전트 문자열에 접근할 수 있다. 
(https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/User-Agent)


User-Agent는 속성이라기보다는 사용자의 신분을 대신할 수 있는 소프트웨어를 말한다. 웹과 관련된 맥락에서는 일반적으로 브라우저 종류나 버전 정보, 호스트의 운영체제 정보를 말한다.
이 헤더는 왜 쓰이나? 바로 호환성을 유지하기 위함이다. 브라우저 전쟁 이후 다양한 브라우저가 생겼는데, 호환성을 유지하기 위해 기존 브라우저 UA에 스트링을 계속 덧붙이다보니, 지금과 같은 끔찍한 형태가 되어 버렸다.
과거에 넷스케이프 브라우저에면 <frame> 태그를 지원했는데, 마이크로소프트가 자신의 UA를 Mozilla로 시작하게 만듦으로서 웹 서버를 속여 <frame> 태그를 지원하도록 했다. (https://wormwlrm.github.io/2021/10/11/Why-User-Agent-string-is-so-complex.html)


#### **3. Connection**
Connection 헤더는 close로 통일하자.

> 👉 Connection 헤더는 현재의 전송이 완료된 이후, 네트워크 접속을 유지할지 말지를 제어한다. 
만일 값이 keep-alive면, 연결은 지속되고 끊기지 않으며 동일한 서버에 대한 후속 요청을 수행할 수 있다. 반면 값이 close이면, 클라이언트 혹은 서버가 연결을 닫으려고 하는 것을 의미한다. HTTP/1.0 요청에서 close가 기본이다.
(https://developer.mozilla.org/ko/docs/Web/HTTP/Headers/Connection)


#### 4. Proxy-Connection
Proxy-Connection 헤더는 close로 통일하자.
Connection, Proxy-Connection 헤더는 처음 request, response 교환이 끝나고도 커넥션이 유지되어야 하는지 아닌지를 명시한다. 지금은 모두 close로 통일되어 있으므로 첫 번째 request, response 교환이 된 이후 커넥션이 바로 닫혀야 한다.
만일 브라우저가 추가적인 요청 헤더를 HTTP request로 보낸다면, 프록시는 이러한 헤더들도 온전히 포함해 서버에게 보내야 한다.


### 1-3. Port number
2개의 포트 넘버가 중요하다. 하나는 HTTP request 포트이고 하나는 Proxy의 listening 포트이다.
1. HTTP request port는 HTTP request의 URL에서 선택적으로 추가할 수 있는 필드이다. 즉, URL에서 포트번호가 적혀 있을 수도 있고 아닐 도 있다. http://www.cmu.edu:8080/hub/index.html 에서 :뒤에 있는 8080이 포트 번호인데, 이는 프록시가 디폴트 HTTP 포트 번호인 80을 사용하지 않고 8080을 사용하고 있는 www.cmu.edu 호스트와 연결해야 한다는 의미이다.

2. listening port는 프록시가 자신과 커넥션을 형성하고자 하는 요청들을 듣기 위한 포트이다. 커맨드 라인에 프록시가 listening port로 사용할 포트 번호를 명시하면 된다. 

1024보다는 크고 65536보다는 작은 포트 번호 중 다른 프로세스가 이미 사용하고 있지 않은 포트 번호를 고르면 된다. 

## Part 2: Dealing with multiple concurrent request
sequential web proxy는 한 번에 하나의 request밖에 처리하지 못했다. 반면 concurrent proxy는 동시에 여러개의 request를 처리할 수 있다. concurrent server를 구현하는 가장 쉬운 방법은 각각의 새로운 커넥션 요청을 처리할 새로운 스레드를 만드는 것이다.


## Part 3: Caching web objects
메모리에 있는 최근에 사용한 web objects를 저장할 캐시를 프록시에 추가해 보자. 
프록시가 서버로부터 웹 객체를 전달받을 때, 클라이언트에게 객체를 전송하는 동시에 메모리에 저장한다. 만일 다르 클라이언트가 같은 서버에게 같은 객체를 요구하면, 프록시는 서버와 다시 커넥션을 형성할 필요 없이 그 캐시해 놓은 객체를 재전달하면 된다.
만일 프록시가 모든 객체를 캐시해 놓는다면 무한한 크기의 메모리가 필요할 것이다. 게다가  웹 객체의 크기는 정해져 있지 않기 때문에 하나의 큰 웹 객체가 전체 캐시를 다 사용해 다른 객체가 캐시되지 못하게 하는 문제를 낳을 수 있다. 이러한 문제를 피하기 위해서 프록시는 캐시 사이즈를 정해야 하고 캐시할 수 있는 객체 크기의 리미트를 정해야 한다.

### 3-1. Maximum cache size
프록시의 캐시 크기는 최대 1MiB라고 정하자.
> 👉 MAX_CACHE_SIZE = 1 MiB
캐시 크기를 계산할 때, 프록시는 실제 웹 객체들을 저장하기 위해 사용하고 있는 바이트만큼을 계산하면 된다. 메타 데이터를 포함한 추가적인 바이트는 무시하라.
3-2. Maximum object size
프록시는 100KiB를 넘지 않는 웹 오브젝트만 캐시할 수 있다.

> 👉 MAX_OBJECT_SIZE = 100 KiB
3-3. Eviction policy
프록시의 캐시는 LRU에 근접한 eviction 정책을 사용한다. 정교한 LRU 정책은 아니지만, 비슷하다. 객체를 읽고 쓰는 것도 그 객체를 사용하는 것만큼이나 중요하다.

### 3-4. Synchronization
캐시에 접근하는 것은 thread-safe해야 하고 캐시에 접근하는 것이 레이스 컨디션으로부터 자유롭도록 보장해야 한다. 여러개의 스레드가 동시에 캐시에서 읽을 수 있도록 구현해야 한다. 당연히 한 번에 하나의 스레드만 캐시에 적을 수 있도록 해야 하지만 이 규칙이 읽기에는 적용되지 않는다.
하나의 락을 사용해 캐시에 접근하는 것을 막는 방식은 이 과제에서 적용되지 않는다. 캐시 분할, 스레드 리드-라이트 락 사용, 세마포어 사용 등의 옵션을 활용해 리더-라이터 문제를 해결해야 한다. 


# 디렉터리 구조
```
####################################################################
# CS:APP Proxy Lab
#
# Student Source Files
####################################################################

This directory contains the files you will need for the CS:APP Proxy
Lab.

proxy.c
csapp.h
csapp.c
    These are starter files.  csapp.c and csapp.h are described in
    your textbook. 

    You may make any changes you like to these files.  And you may
    create and handin any additional files you like.

    Please use `port-for-user.pl' or 'free-port.sh' to generate
    unique ports for your proxy or tiny server. 

Makefile
    This is the makefile that builds the proxy program.  Type "make"
    to build your solution, or "make clean" followed by "make" for a
    fresh build. 

    Type "make handin" to create the tarfile that you will be handing
    in. You can modify it any way you like. Your instructor will use your
    Makefile to build your proxy from source.

port-for-user.pl
    Generates a random port for a particular user
    usage: ./port-for-user.pl <userID>

free-port.sh
    Handy script that identifies an unused TCP port that you can use
    for your proxy or tiny. 
    usage: ./free-port.sh

driver.sh
    The autograder for Basic, Concurrency, and Cache.        
    usage: ./driver.sh

nop-server.py
     helper for the autograder.         

tiny
    Tiny Web server from the CS:APP text
```
