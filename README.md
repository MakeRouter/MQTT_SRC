# MATT_SRC
One step closer to a smart router

## 사용자 인식 ( user awareness.c )

- 사용자가 휴대폰으로 wifi 을 인식하면 5초 이내로 mqtt broker로 사용자의 휴대폰 정보를 전달함
- 단, 등록된 사용자만 인식하고, 나머지는 무시
- 이는 사용자가 어떻게 활용하는지에 따라 사용성이 무궁무진함.
- ex) 내 휴대폰 인식 시, 내 컴퓨터의 노션 창을 띄워준다. ( 일정 관리를 노션으로 하기 때문에 유용하게 사용중 )

<img width="696" height="391" alt="image" src="https://github.com/user-attachments/assets/73e9e478-3fe2-40a2-bec6-cf17b0a0aeb5" />


## 컴퓨터에서 노션 열기 예시 ( 내가 사용중인 코드, win_notion.py)

- 사용자의 휴대폰이 인식되면 노션 열기
- 이미 열려있는 경우 열지 않음

#### 추후 고민해볼 문제
- 한번 연결한 경우 노션이 열리고 나서 이후 다음 날이 되어도 코드는 1회성인 문제,,
- 새벽 6시 기준으로 상태 초기화 생각중,,

