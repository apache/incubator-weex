---
title: Timer
type: references
version: 0.10
---

# Timer
Weex Timer可以用来延时启动一个一次性任务或者重复任务。Timer会尽最大努力提供精确的延时，但是延时可能并不精确，延时时间很可能会超过用户期望的时间。实际上，timer仅仅是为了支持HTML5中的polyfill，*不建议*开发者直接使用timer.

## API
Timer中延时的单位是毫秒，且延时时间应该为一个非负的**int**值(int值最大为0x7FFFFF).如果延时时间为零，会将该任务马上插入任务队列的尾部。对于其他非法值，timer的行为未定义。

### setTimeout(fn, timeout)    
`setTimeout()`会等待指定的时间，然后执行给出的函数。
* 可以使用 `clearTimeout()`去阻止`setTimeout()`运行如果此时对应的`setTimeout()`还没有运行的话。
* 如果需要重复执行任务，使用`setInterval()`.

#### Arguments
* `fn` (function): 待执行的函数.
* `timeout` (number): 执行函数前的等待时间，单位为毫秒。 

#### Return value
一个Number对象, 表示这个任务的id。把id传入clearTimeout(fnId)中可以用来取消任务。   

### setInterval(fn, interval)    
`setInterval()`每隔指定的时间间隔后，会执行对应的函数。`setInterval()`不会停止，除非`clearInterval()`被调用。`setInterval()`的返回值可以用来作为`setInterval()`的参数。

#### Arguments    
* `fn` (function): 待执行的函数。
* `interval` (number): 这次执行函数与下一次函数之间的时间间隔，单位为毫秒。

#### Return value    
一个Number对象，代表任务序列的id。可以把这个值传入`clearInterval`中来终止重复任务的执行。 

### clearTimeout(fnId)
`clearTimeout()`可以用来提前终止`setTimeout()`启动的任务。仅当`setTimeout()`对应的任务没有被执行时，`clearTimeout()`才可以用来终止该任务，否则`clearTimeout()`没有任何效果。

#### Arguments    
* `fnId` (number): `setTimeout()`的返回值. 

### clearInterval(fnId)
 `clearInterval()`可以用来终止 `setInterval()`启动的任务序列。

#### Arguments
* `fnId` (number): `setInterval()`的返回值

[Try it](http://dotwe.org/vue/ad564965f1eac5a4bc86946ecff70a0c)
