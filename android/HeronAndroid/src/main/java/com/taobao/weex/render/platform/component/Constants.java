package com.taobao.weex.render.platform.component;

public interface Constants {

    interface Tag{
        static String VIDEO = "video";
        static String INPUT = "input";
        static String WEB = "web";
        static String TVIDEO = "tvideo";
    }

    interface Attr{
        interface Input{
            String ATTR_PLACEHOLDER = "placeholder";
            String ATTR_TYPE = "type";
            String ATTR_VALUE = "value";
            String ATTR_FONT_SIZE = "font-size";
            String ATTR_AUTO_FOCUS = "autofocus";
            String ATTR_SINGLELINE = "singleline";
            String ATTR_LINES = "lines";
            String ATTR_MAX = "max";
            String ATTR_MIN = "min";
            String ATTR_MAXLENGTH = "maxlength";
            String ATTR_RETURN_KEY = "return-key-type";
            String ATTR_AUTO_CAPITALIZATION_TYPE = "auto-capitalization-type";

            interface Value{
                String TEXT_ALIGN_LEFT = "left";
                String TEXT_ALIGN_RIGHT = "right";
                String TEXT_ALIGN_CENTER = "center";
            }
        }

        interface Web{
            String HREF = "href";
        }

        interface Video{
            String SRC = "src";
        }
    }


    interface Style{
        interface Input{
          String COLOR = "color";
          String FONT_SIZE = "font-size";
          String FONT_STYLE = "font-style";
          String FONT_WEIGHT = "font-weight";
          String TEXT_ALIGN  = "text-align";
        }
    }

    interface Event{
        interface Video{
            String START = "start";
            String PAUSE = "pause";
            String FINISH = "finish";
            String FAIL = "fail";
            String ERROR = "error";
        }
    }

    interface Name{
        interface Video{
            String PLAY_STATUS = "playStatus";
        }
    }

    interface Value{
        interface Video{
            String PLAY = "play";
            String PAUSE = "pause";
            String STOP = "stop";
        }
    }




}
