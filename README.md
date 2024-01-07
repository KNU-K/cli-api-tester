# cli-api-tester

이 프로젝트는 linux server상에서 동작가능한 cli_tester이다. 

http 에 대한 GET,POST, PUT, DELETE 등과 같은 작업이 아무래도 커널 환경에서 동작하기 힘들기 때문에,
curses 와 socket library를 활용하여 사용자가 좀 더 편의성있게 사용할 수 있도록 하였다.


## FUNC
* MakeFile구성으로 build 편의성 증가
* curses 를 통한 cli 상에 그래픽 컨트롤
* socket을 통해서 http 프로토콜로 meta-data를 전송하여 그에 대한 결과가 정상적인지 테스트 가능
