# Тестую CAN

Для тактування 160МГц

```
can timing can@4000a400 1 0 13 2 20
can mode can@4000a400 normal
can start can@4000a400
can send can@4000a400 123 01

can timing can@4000a800 1 0 13 2 20
can mode can@4000a800 normal
can start can@4000a800
can send can@4000a800 123 01
```

Окремо обидва працюють.

Одночасно - ні.