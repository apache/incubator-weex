/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
package com.taobao.weex.ui.component;

import junit.framework.TestCase;

import org.junit.Test;

import java.util.Comparator;
import java.util.PriorityQueue;

/**
 * Created by furture on 2018/1/3.
 */

public class PriorityQueueTest extends TestCase {


    @Test
    public void testPriority(){
        PriorityQueue<Integer> embeds = new PriorityQueue<>(8, new Comparator<Integer>() {
            @Override
            public int compare(Integer o1, Integer o2) {
                return o1 - o2;
            }
        });


        embeds.add(10);
        embeds.add(9);
        embeds.offer(1);

        embeds.offer(2);
        embeds.add(11);

        System.out.println(embeds.peek()  + "  " + embeds.size());

        System.out.println(embeds.poll() + "  " + embeds.size());
    }
}
