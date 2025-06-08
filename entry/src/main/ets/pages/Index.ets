import { hilog } from '@kit.PerformanceAnalysisKit';
import testNapi from 'libentry.so';
import { taskpool } from '@kit.ArkTS';
import { util } from '@kit.ArkTS';

const DOMAIN = 0x0000;

@Concurrent
function worker() {
  return testNapi.read();
}

testNapi.run();

// escape non printable characters
function stringToHex(str: ArrayBuffer) {
  let view: Uint8Array = new Uint8Array(str);
  let hex: string = '';
  for (let i = 0; i < view.length; i++) {
    const byte: number = view[i];
    if (byte >= 127 || byte < 32) {
      hex += '\\x';
      hex += byte.toString(16).padStart(2, '0');
    } else {
      hex += String.fromCharCode(byte);
    }
  }
  return hex;
}

class Style {
  fontColor: number = 0x000000;
  fontWeight: FontWeight = FontWeight.Normal;
}

class Character {
  text: string = " ";
  style: Style = new Style();
}

@Observed
class ObservedArray<T> extends Array<T> {
}

@Entry
@Component
struct Index {
  @State terminal: Array<ObservedArray<Character>> = [new ObservedArray<Character>()];
  @State row: number = 0;
  @State col: number = 0;
  @State input: string = "";
  @State escapeState: number = 0;
  @State buffer: string = "";
  @State currentStyle: Style = new Style();
  scroller: Scroller = new Scroller();

  onPageShow() {
    (async () => {
      let task: taskpool.Task = new taskpool.Task(worker);
      while(1) {
        let content: ArrayBuffer = await taskpool.execute(task) as ArrayBuffer;
        if (content === undefined) {
          break;
        } else if (content.byteLength > 0) {
          hilog.info(DOMAIN, 'testTag', 'Got input: %{public}s', stringToHex(content));
          let view: Uint8Array = new Uint8Array(content);
          for (let i = 0; i < view.length; i++) {
            if (this.escapeState !== 0) {
              if (view[i] === 91) {
                // opening bracket, CSI
                this.escapeState = 2;
              } else if (view[i] === 109) {
                // m: set color
                let parts = this.buffer.split(";");
                for (let part of parts) {
                  if (part === "1") {
                    // set bold
                    this.currentStyle.fontWeight = FontWeight.Bold;
                  } else if (part === "31") {
                    // red foreground
                    this.currentStyle.fontColor = 0xFF0000;
                  } else if (part === "32") {
                    // green foreground
                    this.currentStyle.fontColor = 0x00FF00;
                  } else if (part === "34") {
                    // blue foreground
                    this.currentStyle.fontColor = 0x0000FF;
                  } else if (part === "36") {
                    // cyan foreground
                    this.currentStyle.fontColor = 0x00FFFF;
                  } else if (part === "0") {
                    // reset
                    this.currentStyle = new Style();
                  } else {
                    hilog.info(DOMAIN, 'testTag', 'Unknown color: %{public}s', part);
                  }
                }
                this.escapeState = 0;
              } else if (view[i] === 74) {
                // J
                if (this.buffer === "2") {
                  // clear screen
                  for (let i = 0;i < this.row;i ++) {
                    this.terminal[i] = [];
                  }
                }
                this.escapeState = 0;
              } else if (view[i] === 72) {
                // H, move cursor to upper left corner
                this.row = 0;
                this.col = 0;
                this.escapeState = 0;
              } else {
                this.buffer += String.fromCharCode(view[i]);
              }
              continue;
            }

            if (view[i] >= 32 && view[i] < 127) {
              // printable
              while (this.col >= this.terminal[this.row].length) {
                let ch = new Character;
                this.terminal[this.row].push(ch);
              }
              
              let ch = new Character;
              ch.text = String.fromCharCode(view[i]);
              ch.style.fontColor = this.currentStyle.fontColor;
              ch.style.fontWeight = this.currentStyle.fontWeight;
              this.terminal[this.row][this.col] = ch;
              this.col += 1;
            } else if (view[i] === 0x0d) {
              // carriage return
              this.col = 0;
            } else if (view[i] === 0x0a) {
              // line feed
              this.row += 1;
              while (this.row >= this.terminal.length) {
                this.terminal.push(new ObservedArray<Character>());
              }
            } else if (view[i] === 0x08) {
              // backspace
              while (this.col >= this.terminal[this.row].length) {
                let ch = new Character;
                this.terminal[this.row].push(ch);
              }
              this.terminal[this.row][this.col] = new Character;
              this.col -= 1;
            } else if (view[i] === 0x1b) {
              // escape
              this.buffer = "";
              this.escapeState = 1;
            }
          }
          // dirty trick to make it refresh
          this.terminal.push(this.terminal.pop() as ObservedArray<Character>);
          this.scroller.scrollEdge(Edge.Bottom);
        }
      }
    })();
  }

  build() {
    Row() {
      Column() {
        Scroll(this.scroller) {
          Column() {
            ForEach(this.terminal, (row: ObservedArray<Character>) => {
              Text() {
                ForEach(row, (ch: Character) => {
                  Span(ch.text)
                    .fontColor(ch.style.fontColor)
                    .fontWeight(ch.style.fontWeight)
                })
              }
              .textAlign(TextAlign.Start)
              .fontFamily("monospace")
              .width("100%")
            })
          }
          .align(Alignment.Top)
        }
        .align(Alignment.Top)
        .height("90%")
        Row() {
          TextInput({ text: this.input })
            .onChange((value: string) => {
              this.input = value;
              console.info(value);
            })
            .onSubmit(() => {
              let data = this.input + "\n";
              let textEncoder = util.TextEncoder.create('utf-8');
              let encodeResult = textEncoder.encode(data); 
              testNapi.send(encodeResult.buffer);
              this.input = "";
              focusControl.requestFocus("input");
            })
            .id("input")
            .width("70%")
        }
      }
      .width('100%')
    }
    .height('100%')
  }
}
