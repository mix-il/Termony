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


@Entry
@Component
struct Index {
  @State message: string = "";
  @State input: string = "";
  scroller: Scroller = new Scroller();

  onPageShow() {
    (async () => {
      let task: taskpool.Task = new taskpool.Task(worker);
      while(1) {
        let content: ArrayBuffer = await taskpool.execute(task) as ArrayBuffer;
        if (content === undefined) {
          break;
        } else if (content.byteLength > 0) {
          let textDecoder = util.TextDecoder.create('utf-8');
          let decodeResult = textDecoder.decodeToString(new Uint8Array(content)); 
          hilog.info(DOMAIN, 'testTag', 'Got %{public}s', stringToHex(content));
          this.message += decodeResult;
          this.scroller.scrollEdge(Edge.Bottom);
        }
      }
    })();
  }

  build() {
    Row() {
      Column() {
        Scroll(this.scroller) {
          Text(this.message)
            .align(Alignment.Top)
            .textAlign(TextAlign.Start)
            .fontWeight(FontWeight.Bold)
            .fontFamily("monospace")
            .width("100%")
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
