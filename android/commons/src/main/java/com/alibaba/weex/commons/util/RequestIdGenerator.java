package com.alibaba.weex.commons.util;

import java.util.concurrent.atomic.AtomicInteger;

/**
 * Created by moxun on 17/4/20.
 */

public class RequestIdGenerator {
    private static final AtomicInteger sIdGenerator = new AtomicInteger(0);

    public static int nextRequestId() {
        return sIdGenerator.getAndIncrement();
    }
}
