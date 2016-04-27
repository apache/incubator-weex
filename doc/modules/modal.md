# modal

### Summary

Encuslate of Toast, Dialog, with toast, alert, confirm, prompt method

### API

#### toast

this.$call('modal', 'toast', { 'message': arg1, 'duration': arg2 });

arguments:

* arg1: &lt;string&gt; the message that the toast shows
* arg2: &lt;number&gt; the duration that the toast shows

##### example

```
var  msg = "I am toast!";
var  duration = 2;
this.$call('modal','toast',{'message':msg, 'duration': duration });
```

#### alert

arguments:

* arg1: &lt;string&gt; the message that the alert shows
* arg2: &lt;string&gt; the title of alert button

This method has a callback function whose arguments will be:

* res &lt;object&gt; maybe a parameter object
* res.status &lt;string&gt; introduction about this res.status
* res.data &lt;string&gt; introduction about this res.data

##### example

```
var arg1 = "I am alert!"
var arg2 = "I'm ok"
this.$call('modal','alert',{
  'message':arg1,
  'okTitle':arg2,
}, function(e) {
  nativeLog(e.status + ', ' + e.data)
  //todoing something
});
```
    
#### confirm

arguments:

* arg1: &lt;string&gt; the message that the confirm shows
* arg2: &lt;string&gt; the title of confirm button
* arg3: &lt;string&gt; the title of cancel button

This method has a callback function whose arguments will be:

* res &lt;string&gt; the title of the button the user clicked
  
##### example

```
var arg1 = "I am alert!"
var arg2 = "I'm ok"
var arg3 = "I'm cancel"
this.$call('modal','confirm',{
  'message':arg1,
  'okTitle':arg2,
  'cancelTitle':arg3
}, function(e) {
  nativeLog(e.status + ', ' + e.data)
});
```

#### prompt

arguments:

* arg1: &lt;string&gt; the message that the prompt shows
* arg2: &lt;string&gt; the  title of prompt confirm button
* arg3: &lt;string&gt; the  title of confirm cancel button
   
This method has a callback function whose arguments will be:

* res &lt;object&gt; parameter object
* res.status &lt;string&gt; the title of the button the user clicked
* res.data &lt;string&gt; the value of the text the user entered

##### example

```
var arg1 = "I am prompt!"
var arg2 = "I'm ok"
var arg3 = "I'm cancel"  
this.$call('modal','prompt',{
    'message':arg1,
    'okTitle':arg2,
    'cancelTitle':arg3
}, function(e) {
    nativeLog(e.status + ', ' + e.data)
});
```



