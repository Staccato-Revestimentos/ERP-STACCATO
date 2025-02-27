/************************************************************************
 * file name         : easy_qtimer.h
 * ----------------- :
 * creation time     : 2016/12/05
 * author            : Victor Zarubkin
 * email             : v.s.zarubkin@gmail.com
 * ----------------- :
 * description       : This file contains implementation of EasyQTimer class used to
 *                   : connect QTimer to non-QObject classes.
 * ----------------- :
 * change log        : * 2016/12/05 Victor Zarubkin: Initial commit.
 *                   :
 *                   : *
 * ----------------- :
 * license           : Lightweight profiler library for c++
 *                   : Copyright(C) 2016-2017  Sergey Yagovtsev, Victor Zarubkin
 *                   :
 *                   : Licensed under either of
 *                   :     * MIT license (LICENSE.MIT or http://opensource.org/licenses/MIT)
 *                   :     * Apache License, Version 2.0, (LICENSE.APACHE or http://www.apache.org/licenses/LICENSE-2.0)
 *                   : at your option.
 *                   :
 *                   : The MIT License
 *                   :
 *                   : Permission is hereby granted, free of charge, to any person obtaining a copy
 *                   : of this software and associated documentation files (the "Software"), to deal
 *                   : in the Software without restriction, including without limitation the rights
 *                   : to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 *                   : of the Software, and to permit persons to whom the Software is furnished
 *                   : to do so, subject to the following conditions:
 *                   :
 *                   : The above copyright notice and this permission notice shall be included in all
 *                   : copies or substantial portions of the Software.
 *                   :
 *                   : THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 *                   : INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 *                   : PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *                   : LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 *                   : TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 *                   : USE OR OTHER DEALINGS IN THE SOFTWARE.
 *                   :
 *                   : The Apache License, Version 2.0 (the "License")
 *                   :
 *                   : You may not use this file except in compliance with the License.
 *                   : You may obtain a copy of the License at
 *                   :
 *                   : http://www.apache.org/licenses/LICENSE-2.0
 *                   :
 *                   : Unless required by applicable law or agreed to in writing, software
 *                   : distributed under the License is distributed on an "AS IS" BASIS,
 *                   : WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *                   : See the License for the specific language governing permissions and
 *                   : limitations under the License.
 ************************************************************************/

#include "easy_qtimer.h"

//////////////////////////////////////////////////////////////////////////

EasyQTimer::EasyQTimer() : QObject() {
  connect(&m_timer, &QTimer::timeout, [this]() { m_handler(); });
}

EasyQTimer::EasyQTimer(::std::function<void()> &&_handler, bool _isSignleShot) : QObject(), m_handler(::std::forward<::std::function<void()> &&>(_handler)) {
  m_timer.setSingleShot(_isSignleShot);
  connect(&m_timer, &QTimer::timeout, [this]() { m_handler(); });
}

EasyQTimer::~EasyQTimer() {}

void EasyQTimer::setHandler(::std::function<void()> &&_handler) { m_handler = _handler; }

//////////////////////////////////////////////////////////////////////////
