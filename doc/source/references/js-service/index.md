---
title: JS Service
type: references
order: 9
version: 2.1
---


# JS Service

JS service and Weex instance are parallel in js runtime. Weex instance lifecycle will invoke JS service lifecycle.


## Register JS Service

### iOS
```objective-c
[WeexSDKEngine registerService:@"SERVICE_NAME" withScript: @"SERVICE_JS_CODE" withOptions: @{}];
```

### Android
```java
HashMap<String, String> options = new HashMap<>()
options.put("k1", "v1")
String SERVICE_NAME = "SERVICE_NAME"
String SERVICE_JS_CODE = "SERVICE_JS_CODE"
boolean result = WXSDKEngine.registerService(SERVICE_NAME, SERVICE_JS_CODE, options);
```

### Web
```html
<!-- Should be inject after then jsfm -->
<script src="JS_SERVICE_CODE_URL"></script>
```



## Write a js service
```javascript
// options: native inject options
// options.serviceName is native options name
var modal = weex.requireModule('modal')

service.register(options.serviceName, {

    // service lifecycle. create instance
    create: function(id, env, config) {
        return {
            instance: {
                InstanceService: function(weex) {
                    return {
                        toast: function(title) {
                            modal.toast({ message: title })
                        }
                    }
                }
            },
            NormalService: function(weex) {
                return {
                    toast: function(title) {
                        modal.toast({ message: title })
                    }
                }
            }
        }
    },

    // service lifecycle. refresh instance
    refresh: function(id, env, config){

    },

    // service lifecycle. destroy instance
    destroy: function(id, env) {

    }
})
```

## Using js service (vuejs)
```
<script>
var _InstanceService = new InstanceService(weex)
var _NormalService = new service.normalService(weex)

module.exports = {
	created: fucntion() {
		// called modal module to toast something
		_InstanceService.toast('Instance JS Service')
        _NormalService.toast('Normal JS Service')
	}
}
</script>
```
