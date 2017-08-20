/*
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
export default `
.weex-toast {
  font-size: 0.426667rem;
  line-height: 0.426667rem;
  position: fixed;
  z-index: 1999999999;
  box-sizing: border-box;
  max-width: 80%;
  bottom: 50%;
  left: 50%;
  padding: 0.213333rem;
  background-color: #000;
  color: #fff;
  text-align: center;
  opacity: 0.7;
  -webkit-transition: all 0.4s ease-in-out;
          transition: all 0.4s ease-in-out;
  border-radius: 0.066667rem;
  -webkit-transform: translateX(-50%);
  -ms-transform: translateX(-50%);
      transform: translateX(-50%);
}

.weex-toast.hide {
  opacity: 0;
}

.weex-alert .weex-alert-ok {
  width: 100%;
}

.weex-confirm .btn-group .btn {
  float: left;
  width: 50%;
}

.weex-confirm .btn-group .btn.btn-ok {
  border-right: 0.013333rem solid #ddd;
}

.weex-modal-wrap {
  display: none;
  position: fixed;
  z-index: 999999999;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  background-color: #000;
  opacity: 0.5;
}

.weex-modal-node {
  position: fixed;
  z-index: 9999999999;
  top: 50%;
  left: 50%;
  width: 6.666667rem;
  min-height: 2.666667rem;
  border-radius: 0.066667rem;
  -webkit-transform: translate(-50%, -50%);
  -ms-transform: translate(-50%, -50%);
      transform: translate(-50%, -50%);
  background-color: #fff;
}

.weex-modal-node.hide {
  display: none;
}

.weex-modal-node .content {
  display: -webkit-box;
  display: -webkit-flex;
  display: -ms-flexbox;
  display: flex;
  -webkit-box-orient: vertical;
  -webkit-flex-direction: column;
  -ms-flex-direction: column;
      flex-direction: column;
  -webkit-box-align: center;
  -webkit-align-items: center;
  -ms-flex-align: center;
      align-items: center;
  -webkit-box-pack: center;
  -webkit-justify-content: center;
  -ms-flex-pack: center;
      justify-content: center;
  width: 100%;
  min-height: 1.866667rem;
  box-sizing: border-box;
  font-size: 0.426667rem;
  line-height: 0.426667rem;
  padding: 0.213333rem;
  border-bottom: 0.013333rem solid #ddd;
}

.weex-modal-node .btn-group {
  width: 100%;
  height: 0.8rem;
  font-size: 0.373333rem;
  text-align: center;
  margin: 0;
  padding: 0;
  border: none;
}

.weex-modal-node .btn-group .btn {
  text-align: center;
}

.weex-modal-node .btn-group .btn {
  box-sizing: border-box;
  height: 0.8rem;
  line-height: 0.8rem;
  margin: 0;
  padding: 0;
  border: none;
  background: none;
  text-align: center;
}

.weex-prompt .input-wrap {
  box-sizing: border-box;
  width: 100%;
  margin-top: 0.133333rem;
  height: 0.96rem;
}

.weex-prompt .input-wrap .input {
  box-sizing: border-box;
  width: 100%;
  height: 0.56rem;
  line-height: 0.56rem;
  font-size: 0.426667rem;
  border: 0.013333rem solid #999;
}

.weex-prompt .btn-group .btn {
  float: left;
  width: 50%;
}

.weex-prompt .btn-group .btn.btn-ok {
  border-right: 0.013333rem solid #ddd;
}
`
