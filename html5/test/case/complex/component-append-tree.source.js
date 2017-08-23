define('@weex-component/sub', function(require, exports, module) {
  
      ;
      module.exports = {
          data: function() {
              return {
                  str: '-----'
              }
          },
          ready: function() {
              this.str = 'ready'
              setTimeout(() => {
                  this.str = 'timeout'
              }, 100)
          }
      }
  
  
      ;
      module.exports.style = {}
  
      ;
      module.exports.template = {
          "type": "text",
          "attr": {
              "value": function() {
                  return this.str
              }
          }
      }
  
      ;
  })
  
  // module
  
  define('@weex-component/ad164d40f42747f598b56eccfd0eae8e', function(require, exports, module) {
  
      ;
  
      ;
      module.exports.style = {}
  
      ;
      module.exports.template = {
          "type": "scroller",
          "children": [{
              "type": "div",
              "append": "node",
              "children": [{
                  "type": "sub"
              }]
          }, {
              "type": "div",
              "append": "tree",
              "children": [{
                  "type": "sub"
              }]
          }]
      }
  
      ;
  })
  
  // require module
  bootstrap('@weex-component/ad164d40f42747f598b56eccfd0eae8e', {
      "transformerVersion": "0.4.3"
  })