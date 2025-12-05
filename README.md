# Проект по мрежово програмиране

Мултинишков сървър изпълняващ алгоритъма на Йен върху данни предоставени от клиент.

Име: Мартин Георгиев Георгиев

Факултетен номер: 9MI0800416

Курс: 3

Група: 5

# Компилиране
Проектът е конфигуриран със `cmake`.
За да се компилира на `Linux`:
```
# mkdir build
# cd build
# cmake ..
# make
```

# Примерно използване
## Сървър:
```
❯ ./server
```

## Клиент:
### Малък граф
```
❯ ./client 127.0.0.1
Successfully connected to 127.0.0.1
Input parameters for Yen's algorithm:
Number of vertices: 6
Number of edges: 9
Number of shortest paths to find: 5
Source vertex: 0
Destination vertex: 4
Edge 0: 0 1 7
Edge 1: 0 2 9
Edge 2: 0 5 14
Edge 3: 1 2 10
Edge 4: 1 3 15
Edge 5: 2 3 11
Edge 6: 2 5 2
Edge 7: 3 4 6
Edge 8: 4 5 9
Server result:
0 2 3 4
0 1 3 4
0 1 2 3 4
```

### Средно голям граф
```
❯ ./client 127.0.0.1
Successfully connected to 127.0.0.1
Input parameters for Yen's algorithm:
Number of vertices: 8
Number of edges: 14
Number of shortest paths to find: 10
Source vertex: 0
Destination vertex: 7
Edge 0: 0 1 4
Edge 1: 0 2 3
Edge 2: 0 4 8
Edge 3: 1 3 5
Edge 4: 1 5 7
Edge 5: 2 3 4
Edge 6: 2 6 10
Edge 7: 3 4 2
Edge 8: 3 6 6
Edge 9: 4 7 9
Edge 10: 5 6 1
Edge 11: 5 7 12
Edge 12: 6 7 3
Edge 13: 2 5 8
Server result:
0 2 5 6 7
0 1 5 6 7
0 2 6 7
0 2 3 6 7
0 4 7
0 1 3 6 7
0 2 3 4 7
0 1 3 4 7
0 1 5 7
0 2 5 7

```

### Плътен граф
```
❯ ./client 127.0.0.1
Successfully connected to 127.0.0.1
Input parameters for Yen's algorithm:
Number of vertices: 7
Number of edges: 20
Number of shortest paths to find: 10
Source vertex: 0
Destination vertex: 6
Edge 0: 0 1 2
Edge 1: 0 2 5
Edge 2: 0 3 6
Edge 3: 0 4 3
Edge 4: 0 5 9
Edge 5: 0 6 1
Edge 6: 1 2 4
Edge 7: 1 3 2
Edge 8: 1 4 7
Edge 9: 1 5 5
Edge 10: 1 6 3
Edge 11: 2 3 1
Edge 12: 2 4 8
Edge 13: 2 5 6
Edge 14: 3 4 2
Edge 15: 3 5 4
Edge 16: 4 5 3
Edge 17: 4 6 5
Edge 18: 5 6 7
Edge 19: 2 6 2
Server result:
0 6
0 1 6
0 2 6
0 1 2 6
0 4 6
0 1 3 4 6
0 2 3 4 6
0 3 4 6
0 4 5 6
0 1 2 3 4 6
```

# Възможни подобрения

Сегашният дизайн на проекта е умишлено минималистичен с фокус върху правилната комуникация между клиента и сървъра.
Ето няколко възможни подобрения, които могат да се имплементират в бъдеще:
* Асинхроненна мултинишков среда за изпълнение -
В момента сървърът ни използва много нишки, но всяка от тези нишки блокира при заявка за четене или писане.
За да се справят с този проблем, съвременните сървъри използват асинхронни среди за изпълнение -
в C++ могат да се използват библиотеки като Boost.Asio.
* Unit тестване на алгоритмите на Дийкстра и Йен
* Подобряване на потребителския интерфейс
* Оптимизации - алгоритмите на Дийкстра и Йен могат да се оптимизират значително,
сегашните им имплементации са опростени, за да се избегнат бъгове.
