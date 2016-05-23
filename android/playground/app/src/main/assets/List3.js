define('@weex-component/TC_Monitor_List_WithAppendTree', function (require, exports, module) {

;
  module.exports = {
    data: function () {return {
      shopList:[
          {
              scopeValue:1,
              PersonPhoto:'https://gw.alicdn.com/tps/i3/TB1yeWeIFXXXXX5XFXXuAZJYXXX-210-210.png_60x60.jpg',
              PersonName:'张三丰',
              PersonVisitTime:'昨天', 
              shopDesc:'极有家，过我想要过的生活，特卖大放送！～～～～～',
              shopImgList:[{shopImgWidth:220,shopImgHeight:220,shopImg:'https://gd2.alicdn.com/bao/uploaded/i2/TB1rtOnHpXXXXXLaXXXXXXXXXXX_!!0-item_pic.jpg_220x220.jpg'},{shopImgWidth:220,shopImgHeight:220,shopImg:'https://gd4.alicdn.com/bao/uploaded/i4/TB15KrfFVXXXXXJXXXXXXXXXXXX_!!0-item_pic.jpg_220x220.jpg'},{shopImgWidth:220,shopImgHeight:220,shopImg:'https://gd2.alicdn.com/bao/uploaded/i2/TB1t5YBGVXXXXXcaXXXXXXXXXXX_!!0-item_pic.jpg_220x220.jpg'}],
              shopLikeImg:'https://img.alicdn.com/tps/i1/TB1jTxXIVXXXXb8XXXX07tlTXXX-200-200.png_88x88xz.jpg',
              shopLikeText:'6',
              shopCommentImg:'https://img.alicdn.com/tps/i1/TB1kTKyIVXXXXacXFXX07tlTXXX-200-200.png_88x88xz.jpg',
              shopCommentText:'97',
              shopLookImg:'https://img.alicdn.com/imgextra/i4/397746073/TB2IseKeFXXXXcVXXXXXXXXXXXX-397746073.png_88x88xz.jpg',
              shopLookText:'1003',
              shareImg:'https://cbu01.alicdn.com/cms/upload/2015/930/224/2422039_702806130.png_88x88xz.jpg',
              shareText:"10",
              
          },
          {
              scopeValue:2,
              PersonPhoto:'https://gw.alicdn.com/tps/i3/TB1yeWeIFXXXXX5XFXXuAZJYXXX-210-210.png_60x60.jpg',
              PersonName:'张无忌',
              PersonVisitTime:'昨天', 
              shopDesc:'女士包包，自由自在！～～～～～',
              shopImgList:[{shopImgWidth:220,shopImgHeight:220,shopImg:'https://gd1.alicdn.com/imgextra/i1/2655929383/TB2.qITjpXXXXcIXXXXXXXXXXXX_!!2655929383.jpg_220x220.jpg'},{shopImgWidth:220,shopImgHeight:220,shopImg:'https://gd3.alicdn.com/imgextra/i3/2655929383/TB2eWwZjpXXXXbHXXXXXXXXXXXX_!!2655929383.jpg_220x220.jpg'},{shopImgWidth:220,shopImgHeight:220,shopImg:'https://gd2.alicdn.com/imgextra/i2/2655929383/TB2tgQWjpXXXXbZXXXXXXXXXXXX_!!2655929383.jpg_220x220.jpg'}],
              shopLikeImg:'https://img.alicdn.com/tps/i1/TB1jTxXIVXXXXb8XXXX07tlTXXX-200-200.png_88x88xz.jpg',
              shopLikeText:'6',
              shopCommentImg:'https://img.alicdn.com/tps/i1/TB1kTKyIVXXXXacXFXX07tlTXXX-200-200.png_88x88xz.jpg',
              shopCommentText:'97',
              shopLookImg:'https://img.alicdn.com/imgextra/i4/397746073/TB2IseKeFXXXXcVXXXXXXXXXXXX-397746073.png_88x88xz.jpg',
              shopLookText:'1003',
              shareImg:'https://cbu01.alicdn.com/cms/upload/2015/930/224/2422039_702806130.png_88x88xz.jpg',
              shareText:"10",
              
          },
          {
              scopeValue:3,
              PersonPhoto:'https://gw.alicdn.com/tps/i3/TB1yeWeIFXXXXX5XFXXuAZJYXXX-210-210.png_60x60.jpg',
              PersonName:'杨过',
              PersonVisitTime:'昨天', 
              shopDesc:'一双好鞋，陪你走遍天涯！～～～～～',
              shopImgList:[{shopImgWidth:220,shopImgHeight:220,shopImg:'https://gd1.alicdn.com/imgextra/i1/TB1AFz9LXXXXXbrXVXXXXXXXXXX_!!0-item_pic.jpg_220x220.jpg'},{shopImgWidth:220,shopImgHeight:220,shopImg:'https://gd2.alicdn.com/imgextra/i2/2557954751/TB2is2njXXXXXatXpXXXXXXXXXX_!!2557954751.jpg_220x220.jpg'},{shopImgWidth:220,shopImgHeight:220,shopImg:'https://gd2.alicdn.com/imgextra/i2/2557954751/TB2PNYGjXXXXXbXXXXXXXXXXXXX_!!2557954751.jpg_220x220.jpg'}],
              shopLikeImg:'https://img.alicdn.com/tps/i1/TB1jTxXIVXXXXb8XXXX07tlTXXX-200-200.png_88x88xz.jpg',
              shopLikeText:'6',
              shopCommentImg:'https://img.alicdn.com/tps/i1/TB1kTKyIVXXXXacXFXX07tlTXXX-200-200.png_88x88xz.jpg',
              shopCommentText:'97',
              shopLookImg:'https://img.alicdn.com/imgextra/i4/397746073/TB2IseKeFXXXXcVXXXXXXXXXXXX-397746073.png_88x88xz.jpg',
              shopLookText:'1003',
              shareImg:'https://cbu01.alicdn.com/cms/upload/2015/930/224/2422039_702806130.png_88x88xz.jpg',
              shareText:"10",
              
          }
      ],
    }},
    methods: {
      loadmore:function (e) {
        
      }
    }
    
  }


;module.exports.style = {
  "flexRow": {
    "flexDirection": "row"
  },
  "flexColumn": {
    "flexDirection": "column"
  },
  "shopImg": {
    "width": 220,
    "height": 220
  },
  "list": {
    "flexDirection": "column",
    "overflow": "hidden",
    "width": 750,
    "height": 1200,
    "backgroundColor": "#dddddd"
  },
  "cell": {
    "backgroundColor": "#dddddd",
    "flexDirection": "column",
    "width": 750
  },
  "shopDiv": {
    "flexDirection": "column",
    "backgroundColor": "#ffffff",
    "margin": 5,
    "padding": 10,
    "borderWidth": 1,
    "borderColor": "#cccccc",
    "overflow": "visible"
  },
  "shopRowList": {
    "flexDirection": "column",
    "borderWidth": 1,
    "borderColor": "#cccccc",
    "overflow": "visible",
    "margin": 5,
    "padding": 10,
    "backgroundColor": "#ffffff"
  },
  "shopHeader": {
    "flexDirection": "row",
    "width": 720
  },
  "shopFooter": {
    "flexDirection": "row",
    "width": 720
  },
  "smallImg": {
    "width": 20,
    "height": 20
  }
}

;module.exports.template = {
  "type": "div",
  "style": {
    "flexDirection": "column",
    "backgroundColor": "#dddddd"
  },
  "children": [
    {
      "type": "list",
      "classList": [
        "list"
      ],
      "events": {
        "loadmore": "loadmore"
      },
      "attr": {
        "loadmoreoffset": "2000"
      },
      "children": [
        {
          "type": "cell",
          "append": "tree",
          "classList": [
            "cell"
          ],
          "repeat": function () {return this.shopList},
          "attr": {
            "scope": function () {return this.scopeValue}
          },
          "children": [
            {
              "type": "div",
              "classList": [
                "shopDiv"
              ],
              "children": [
                {
                  "type": "div",
                  "classList": [
                    "shopHeader"
                  ],
                  "style": {
                    "flexDirection": "row"
                  },
                  "children": [
                    {
                      "type": "div",
                      "style": {
                        "flex": 2,
                        "flexDirection": "row"
                      },
                      "children": [
                        {
                          "type": "div",
                          "children": [
                            {
                              "type": "image",
                              "style": {
                                "width": 60,
                                "height": 60
                              },
                              "attr": {
                                "src": function () {return this.PersonPhoto}
                              }
                            }
                          ]
                        },
                        {
                          "type": "div",
                          "style": {
                            "flexDirection": "column",
                            "marginLeft": 5
                          },
                          "children": [
                            {
                              "type": "div",
                              "style": {
                                "marginTop": 5
                              },
                              "children": [
                                {
                                  "type": "text",
                                  "style": {
                                    "fontSize": 20
                                  },
                                  "attr": {
                                    "value": function () {return this.PersonName}
                                  }
                                }
                              ]
                            },
                            {
                              "type": "div",
                              "style": {
                                "marginTop": 5
                              },
                              "children": [
                                {
                                  "type": "text",
                                  "style": {
                                    "fontSize": 20,
                                    "color": "#cccccc"
                                  },
                                  "attr": {
                                    "value": function () {return this.PersonVisitTime}
                                  }
                                }
                              ]
                            }
                          ]
                        }
                      ]
                    },
                    {
                      "type": "div",
                      "style": {
                        "flex": 5,
                        "flexDirection": "row"
                      },
                      "children": [
                        {
                          "type": "div",
                          "style": {
                            "flexDirection": "row",
                            "marginLeft": 10,
                            "marginTop": 10
                          },
                          "children": [
                            {
                              "type": "div",
                              "children": [
                                {
                                  "type": "image",
                                  "style": {
                                    "width": 20,
                                    "height": 20
                                  },
                                  "attr": {
                                    "src": "https://img.alicdn.com/tps/i4/TB1zkDeIFXXXXXrXVXX07tlTXXX-200-200.png_88x88xz.jpg"
                                  }
                                }
                              ]
                            },
                            {
                              "type": "div",
                              "children": [
                                {
                                  "type": "text",
                                  "style": {
                                    "fontSize": 20
                                  },
                                  "attr": {
                                    "value": "希望"
                                  }
                                }
                              ]
                            }
                          ]
                        },
                        {
                          "type": "div",
                          "style": {
                            "position": "absolute",
                            "left": 90,
                            "top": 10,
                            "flexDirection": "row"
                          },
                          "children": [
                            {
                              "type": "div",
                              "children": [
                                {
                                  "type": "image",
                                  "style": {
                                    "width": 20,
                                    "height": 20
                                  },
                                  "attr": {
                                    "src": "https://img.alicdn.com/tps/i2/TB1hRb1IXXXXXX3XVXXXQaP.pXX-87-87.jpeg"
                                  }
                                }
                              ]
                            },
                            {
                              "type": "div",
                              "children": [
                                {
                                  "type": "text",
                                  "style": {
                                    "fontSize": 20
                                  },
                                  "attr": {
                                    "value": "会员"
                                  }
                                }
                              ]
                            }
                          ]
                        },
                        {
                          "type": "div",
                          "style": {
                            "position": "absolute",
                            "left": 170,
                            "top": 10,
                            "flexDirection": "row"
                          },
                          "children": [
                            {
                              "type": "div",
                              "children": [
                                {
                                  "type": "image",
                                  "style": {
                                    "width": 20,
                                    "height": 20
                                  },
                                  "attr": {
                                    "src": "https://img.alicdn.com/tps/i3/TB1DGkJJFXXXXaZXFXX07tlTXXX-200-200.png"
                                  }
                                }
                              ]
                            },
                            {
                              "type": "div",
                              "children": [
                                {
                                  "type": "text",
                                  "style": {
                                    "fontSize": 20
                                  },
                                  "attr": {
                                    "value": "认证"
                                  }
                                }
                              ]
                            }
                          ]
                        },
                        {
                          "type": "div",
                          "style": {
                            "position": "absolute",
                            "left": 250,
                            "top": 10,
                            "flexDirection": "row"
                          },
                          "children": [
                            {
                              "type": "div",
                              "children": [
                                {
                                  "type": "image",
                                  "style": {
                                    "width": 20,
                                    "height": 20
                                  },
                                  "attr": {
                                    "src": "https://img.alicdn.com/tps/i1/TB1nFvPIXXXXXbUXXXXUAkPJpXX-87-87.png"
                                  }
                                }
                              ]
                            },
                            {
                              "type": "div",
                              "children": [
                                {
                                  "type": "text",
                                  "style": {
                                    "fontSize": 20
                                  },
                                  "attr": {
                                    "value": "加友"
                                  }
                                }
                              ]
                            }
                          ]
                        },
                        {
                          "type": "div",
                          "style": {
                            "position": "absolute",
                            "left": 330,
                            "top": 10,
                            "flexDirection": "row"
                          },
                          "children": [
                            {
                              "type": "div",
                              "children": [
                                {
                                  "type": "image",
                                  "style": {
                                    "width": 20,
                                    "height": 20
                                  },
                                  "attr": {
                                    "src": "https://img.alicdn.com/tps/i2/TB1CpD7IXXXXXbSXXXXUAkPJpXX-87-87.png"
                                  }
                                }
                              ]
                            },
                            {
                              "type": "div",
                              "children": [
                                {
                                  "type": "text",
                                  "style": {
                                    "fontSize": 20
                                  },
                                  "attr": {
                                    "value": "留言"
                                  }
                                }
                              ]
                            }
                          ]
                        },
                        {
                          "type": "div",
                          "style": {
                            "position": "absolute",
                            "left": 410,
                            "top": 10,
                            "flexDirection": "row"
                          },
                          "children": [
                            {
                              "type": "div",
                              "children": [
                                {
                                  "type": "image",
                                  "style": {
                                    "width": 20,
                                    "height": 20
                                  },
                                  "attr": {
                                    "src": "https://gtms02.alicdn.com/tps/i2/TB11ZZfIVXXXXbMXFXXEDhGGXXX-32-32.png"
                                  }
                                }
                              ]
                            },
                            {
                              "type": "div",
                              "children": [
                                {
                                  "type": "text",
                                  "style": {
                                    "fontSize": 20
                                  },
                                  "attr": {
                                    "value": "达人"
                                  }
                                }
                              ]
                            }
                          ]
                        }
                      ]
                    }
                  ]
                },
                {
                  "type": "div",
                  "classList": [
                    "shopBody"
                  ],
                  "style": {
                    "flexDirection": "column"
                  },
                  "children": [
                    {
                      "type": "div",
                      "classList": [
                        "descDiv"
                      ],
                      "style": {
                        "marginTop": 10,
                        "marginLeft": 10
                      },
                      "children": [
                        {
                          "type": "text",
                          "classList": [
                            "shopDesc"
                          ],
                          "style": {
                            "fontSize": 25
                          },
                          "attr": {
                            "value": function () {return this.shopDesc}
                          }
                        }
                      ]
                    },
                    {
                      "type": "div",
                      "style": {
                        "flexDirection": "row"
                      },
                      "children": [
                        {
                          "type": "div",
                          "classList": [
                            "imgDiv"
                          ],
                          "style": {
                            "flex": 1,
                            "flexDirection": "column",
                            "margin": 10
                          },
                          "repeat": function () {return this.shopImgList},
                          "children": [
                            {
                              "type": "div",
                              "children": [
                                {
                                  "type": "image",
                                  "classList": [
                                    "shopImg"
                                  ],
                                  "style": {
                                    "width": function () {return this.shopImgWidth},
                                    "height": function () {return this.shopImgHeight}
                                  },
                                  "attr": {
                                    "src": function () {return this.shopImg}
                                  }
                                }
                              ]
                            },
                            {
                              "type": "div",
                              "style": {
                                "flex": 1,
                                "marginTop": 10,
                                "flexDirection": "row"
                              },
                              "children": [
                                {
                                  "type": "div",
                                  "style": {
                                    "flex": 1,
                                    "justifyContent": "center",
                                    "flexDirection": "row"
                                  },
                                  "children": [
                                    {
                                      "type": "div",
                                      "children": [
                                        {
                                          "type": "image",
                                          "style": {
                                            "width": 20,
                                            "height": 20
                                          },
                                          "attr": {
                                            "src": "https://img.alicdn.com/tps/i1/TB1nFvPIXXXXXbUXXXXUAkPJpXX-87-87.png"
                                          }
                                        }
                                      ]
                                    },
                                    {
                                      "type": "div",
                                      "style": {
                                        "marginLeft": 5
                                      },
                                      "children": [
                                        {
                                          "type": "text",
                                          "style": {
                                            "fontSize": 20,
                                            "textAlign": "center"
                                          },
                                          "attr": {
                                            "value": "喜欢"
                                          }
                                        }
                                      ]
                                    }
                                  ]
                                },
                                {
                                  "type": "div",
                                  "style": {
                                    "flex": 1,
                                    "justifyContent": "center",
                                    "flexDirection": "row"
                                  },
                                  "children": [
                                    {
                                      "type": "div",
                                      "children": [
                                        {
                                          "type": "image",
                                          "style": {
                                            "width": 20,
                                            "height": 20
                                          },
                                          "attr": {
                                            "src": "https://img.alicdn.com/tps/i2/TB1CpD7IXXXXXbSXXXXUAkPJpXX-87-87.png"
                                          }
                                        }
                                      ]
                                    },
                                    {
                                      "type": "div",
                                      "style": {
                                        "marginLeft": 5
                                      },
                                      "children": [
                                        {
                                          "type": "text",
                                          "style": {
                                            "fontSize": 20,
                                            "textAlign": "center"
                                          },
                                          "attr": {
                                            "value": "不爱"
                                          }
                                        }
                                      ]
                                    }
                                  ]
                                }
                              ]
                            }
                          ]
                        }
                      ]
                    }
                  ]
                },
                {
                  "type": "div",
                  "classList": [
                    "shopFooter"
                  ],
                  "style": {
                    "flexDirection": "row",
                    "marginTop": 10,
                    "marginLeft": 10,
                    "justifyContent": "flex-end"
                  },
                  "children": [
                    {
                      "type": "div",
                      "style": {
                        "marginRight": 5
                      },
                      "children": [
                        {
                          "type": "image",
                          "classList": [
                            "shopLikeImg",
                            "smallImg"
                          ],
                          "attr": {
                            "src": function () {return this.shopLikeImg}
                          }
                        }
                      ]
                    },
                    {
                      "type": "div",
                      "style": {
                        "marginRight": 15
                      },
                      "children": [
                        {
                          "type": "text",
                          "classList": [
                            "shopLikeText"
                          ],
                          "style": {
                            "fontSize": 20
                          },
                          "attr": {
                            "value": function () {return this.shopLikeText}
                          }
                        }
                      ]
                    },
                    {
                      "type": "div",
                      "style": {
                        "marginRight": 5
                      },
                      "children": [
                        {
                          "type": "image",
                          "classList": [
                            "shopCommentImg",
                            "smallImg"
                          ],
                          "attr": {
                            "src": function () {return this.shopCommentImg}
                          }
                        }
                      ]
                    },
                    {
                      "type": "div",
                      "style": {
                        "marginRight": 15
                      },
                      "children": [
                        {
                          "type": "text",
                          "classList": [
                            "shopCommentText"
                          ],
                          "style": {
                            "fontSize": 20
                          },
                          "attr": {
                            "value": function () {return this.shopCommentText}
                          }
                        }
                      ]
                    },
                    {
                      "type": "div",
                      "style": {
                        "marginRight": 5
                      },
                      "children": [
                        {
                          "type": "image",
                          "classList": [
                            "shopLookImg",
                            "smallImg"
                          ],
                          "attr": {
                            "src": function () {return this.shopLookImg}
                          }
                        }
                      ]
                    },
                    {
                      "type": "div",
                      "style": {
                        "marginRight": 15
                      },
                      "children": [
                        {
                          "type": "text",
                          "classList": [
                            "shopLookText"
                          ],
                          "style": {
                            "fontSize": 20
                          },
                          "attr": {
                            "value": function () {return this.shopLookText}
                          }
                        }
                      ]
                    },
                    {
                      "type": "div",
                      "style": {
                        "marginRight": 5
                      },
                      "children": [
                        {
                          "type": "image",
                          "classList": [
                            "shareImg",
                            "smallImg"
                          ],
                          "attr": {
                            "src": function () {return this.shareImg}
                          }
                        }
                      ]
                    },
                    {
                      "type": "div",
                      "style": {
                        "marginRight": 20
                      },
                      "children": [
                        {
                          "type": "text",
                          "classList": [
                            "shareText"
                          ],
                          "style": {
                            "fontSize": 20
                          },
                          "attr": {
                            "value": function () {return this.shareText}
                          }
                        }
                      ]
                    }
                  ]
                }
              ]
            }
          ]
        }
      ]
    }
  ]
}

;})

// require module
bootstrap('@weex-component/TC_Monitor_List_WithAppendTree', {"transformerVersion":"0.1.8"})