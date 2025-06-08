import { hilog } from '@kit.PerformanceAnalysisKit';
import testNapi from 'libentry.so';
import { taskpool } from '@kit.ArkTS';
import { util } from '@kit.ArkTS';
import fs from '@ohos.file.fs';

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

class MyXComponentController extends XComponentController {
  onSurfaceCreated(surfaceId: string): void {
    hilog.info(DOMAIN, 'testTag', 'onSurfaceCreated surfaceId: %{public}s', surfaceId);
    testNapi.createSurface(BigInt(surfaceId));
  }

  onSurfaceChanged(surfaceId: string, rect: SurfaceRect): void {
    hilog.info(DOMAIN, 'testTag', 'onSurfaceChanged surfaceId: %{public}s rect: %{public}s', surfaceId, JSON.stringify(rect));
  }

  onSurfaceDestroyed(surfaceId: string): void {
    hilog.info(DOMAIN, 'testTag', 'onSurfaceDestroyed surfaceId: %{public}s', surfaceId);
    testNapi.destroySurface(BigInt(surfaceId))
  }
}

@Entry
@Component
struct Index {
  @State leftCtrlPressed: boolean = false;
  scroller: Scroller = new Scroller();
  xComponentController: XComponentController = new MyXComponentController();

  onPageShow() {
    (async () => {
      let task: taskpool.Task = new taskpool.Task(worker);
      while(1) {
        let content: ArrayBuffer = await taskpool.execute(task) as ArrayBuffer;
        if (content === undefined) {
          break;
        } else if (content.byteLength > 0) {
          hilog.info(DOMAIN, 'testTag', 'Got input: %{public}s', stringToHex(content));
          testNapi.redraw();
        }
      }
    })();
  }

  build() {
    Row() {
      Column() {
        XComponent({
          type: XComponentType.SURFACE,
          controller: this.xComponentController
        })
      }
      .width('100%')
    }
    .height('100%')
    .onKeyEvent((event: KeyEvent) => {
      hilog.info(DOMAIN, 'testTag', 'Got key: %{public}s', JSON.stringify(event));
      if (event.type === KeyType.Down) {
        if (this.leftCtrlPressed && event.unicode as number >= 97 && event.unicode as number <= 122) {
          // Ctrl-A to Ctrl-Z
          let buffer = new ArrayBuffer(1);
          let view = new Uint8Array(buffer);
          view[0] = event.unicode as number - 97 + 1; // ^A is 0x1
          testNapi.send(buffer);
        } else if (event.unicode !== 0) {
          let textEncoder = util.TextEncoder.create('utf-8');
          let encodeResult = textEncoder.encode(String.fromCharCode(event.unicode as number)); 
          testNapi.send(encodeResult.buffer);
        } else {
          let buffer = new ArrayBuffer(1);
          let view = new Uint8Array(buffer);
          if (event.keyText === "KEYCODE_SPACE") {
            view[0] = 0x20;
            testNapi.send(buffer);
          } else if (event.keyText === "KEYCODE_ENTER") {
            view[0] = 0x0d;
            testNapi.send(buffer);
          } else if (event.keyText === "KEYCODE_DEL") {
            view[0] = 0x7f;
            testNapi.send(buffer);
          } else if (event.keyText === "KEYCODE_TAB") {
            view[0] = 0x09;
            testNapi.send(buffer);
          } else if (event.keyText === "KEYCODE_CTRL_LEFT") {
            this.leftCtrlPressed = true;
          }
        }
      } else if (event.type === KeyType.Up) {
        if (event.keyText === "KEYCODE_CTRL_LEFT") {
          this.leftCtrlPressed = false;
        }
      }
    })
  }
}
