/**
 * The MIT License
 *
 * Copyright (c) 2016 Fabio Massaioli
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "../braille.hpp"
#include "../layout.hpp"

#include <cmath>
#include <csignal>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>


#include <random>


using namespace plot;

static volatile std::sig_atomic_t cntr_c_not_encountered = true;

int main() {
  std::signal(SIGINT, [](int) {
    cntr_c_not_encountered = false;
  });

  TerminalInfo term;
  term.detect();

  BrailleCanvas canvas({ 30, 7 }, term);
  auto layout = margin(frame(&canvas, term));

  Rect rect({ 0, 0 }, canvas.size() - Point(1, 2));
  auto size = rect.size() + Point(1, 1);

  auto y0 = rect.p1.y;
  const auto A = size.y/2;
  const auto N = size.x;

  std::vector<float> myValues(N,0.0);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dis(-1, 1);

  auto stroke_fn = [&myValues,&dis,&gen, N, A,y0](const float t){
    static int cc = 0;
    myValues[cc] += dis(gen)*t*A-y0;
    cc = (cc +1) % N;

    return [myValues](const int x){
      Coord base = myValues[x%myValues.size()];
      Coord  end  = myValues[(x+1)%myValues.size()];
      return (base != end) ? std::make_pair(base, end) : std::make_pair(base, base+1); ;
    };
  };


  float t = 0.0f;
  constexpr Color firstColor{ 0.2f, 0.2f, 1.0f };
  while (true) {
    canvas.clear();
    //Plot the sin wave
    canvas.stroke(firstColor, rect, stroke_fn(t));
    //Plot the center horizonal line at 0.0
    canvas.line(term.foreground_color, { rect.p1.x, y0 + A }, { rect.p2.x, y0 + A }, TerminalOp::ClipSrc);
    canvas.line(term.foreground_color, { rect.p1.x, y0 + A }, { rect.p2.x, y0 + A }, TerminalOp::ClipSrc);

    for (auto const& line: layout)
      std::cout << term.clear_line() << line << '\n';

    std::cout << std::flush;

    if (!cntr_c_not_encountered)
      break;

    using namespace std::chrono_literals;
    std::this_thread::sleep_for(40ms);

    if (!cntr_c_not_encountered)
      break;

    t += 0.01f;
    if (t >= 1.0f)
      t -= std::trunc(t);

    std::cout << term.move_up(layout.size().y) << std::flush;
  }

  return 0;
}
