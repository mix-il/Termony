import { hilog } from '@kit.PerformanceAnalysisKit';
import testNapi from 'libentry.so';
import { taskpool } from '@kit.ArkTS';
import { util } from '@kit.ArkTS';
import fs from '@ohos.file.fs';
import { pasteboard } from '@kit.BasicServicesKit';
import { abilityAccessCtrl, bundleManager, Permissions } from '@kit.AbilityKit';
import promptAction from '@ohos.promptAction';

const DOMAIN = 0x0000;

testNapi.run();

class MyXComponentController extends XComponentController {
  onSurfaceCreated(surfaceId: string): void {
    hilog.info(DOMAIN, 'testTag', 'onSurfaceCreated surfaceId: %{public}s', surfaceId);
    testNapi.createSurface(BigInt(surfaceId));
  }

  onSurfaceChanged(surfaceId: string, rect: SurfaceRect): void {
    hilog.info(DOMAIN, 'testTag', 'onSurfaceChanged surfaceId: %{public}s rect: %{public}s', surfaceId, JSON.stringify(rect));
    testNapi.resizeSurface(BigInt(surfaceId), rect.surfaceWidth, rect.surfaceHeight);
  }

  onSurfaceDestroyed(surfaceId: string): void {
    hilog.info(DOMAIN, 'testTag', 'onSurfaceDestroyed surfaceId: %{public}s', surfaceId);
    testNapi.destroySurface(BigInt(surfaceId))
  }
}

let keyMapping: Map<string, number[]> = new Map();
keyMapping.set("KEYCODE_SPACE", [0x20]);
keyMapping.set("KEYCODE_ENTER", [0x0d]);
keyMapping.set("KEYCODE_DEL", [0x7f]);
keyMapping.set("KEYCODE_TAB", [0x09]);
keyMapping.set("KEYCODE_ESCAPE", [0x1b]);
keyMapping.set("KEYCODE_DPAD_UP", [0x1b, 0x5b, 0x41]);
keyMapping.set("KEYCODE_DPAD_DOWN", [0x1b, 0x5b, 0x42]);
keyMapping.set("KEYCODE_DPAD_RIGHT", [0x1b, 0x5b, 0x43]);
keyMapping.set("KEYCODE_DPAD_LEFT", [0x1b, 0x5b, 0x44]);
keyMapping.set("KEYCODE_F1", [0x1b, 0x4f, 0x50]);
keyMapping.set("KEYCODE_F2", [0x1b, 0x4f, 0x51]);
keyMapping.set("KEYCODE_F3", [0x1b, 0x4f, 0x52]);
keyMapping.set("KEYCODE_F4", [0x1b, 0x4f, 0x53]);
keyMapping.set("KEYCODE_F5", [0x1b, 0x5b, 0x31, 0x35, 0x7e]);
keyMapping.set("KEYCODE_F6", [0x1b, 0x5b, 0x31, 0x37, 0x7e]);
keyMapping.set("KEYCODE_F7", [0x1b, 0x5b, 0x31, 0x38, 0x7e]);
keyMapping.set("KEYCODE_F8", [0x1b, 0x5b, 0x31, 0x39, 0x7e]);
keyMapping.set("KEYCODE_F10", [0x1b, 0x5b, 0x32, 0x31, 0x7e]);
keyMapping.set("KEYCODE_F11", [0x1b, 0x5b, 0x32, 0x33, 0x7e]);
keyMapping.set("KEYCODE_F12", [0x1b, 0x5b, 0x32, 0x34, 0x7e]);

@Entry
@Component
struct Index {
  @State leftCtrlPressed: boolean = false;
  @State touchState: Map<number, number> = new Map();
  scroller: Scroller = new Scroller();
  xComponentController: XComponentController = new MyXComponentController();
  timer: number = -1;

  onPageShow() {
    this.timer = setInterval(() => {
      let res: string | undefined = testNapi.checkCopy();
      if (res !== undefined) {
        hilog.info(DOMAIN, 'testTag', 'Copy to pasteboard in ArkTS: %{public}s', res);
        let base64 = new util.Base64Helper();
        let data = base64.decodeSync(res);
        let textDecoder = util.TextDecoder.create('utf-8');
        let result = textDecoder.decode(data);
        hilog.info(DOMAIN, 'testTag', 'Copy to pasteboard in ArkTS decoded: %{public}s', result);
        let pasteData: pasteboard.PasteData = pasteboard.createData(pasteboard.MIMETYPE_TEXT_PLAIN, result);
        let systemPasteboard: pasteboard.SystemPasteboard = pasteboard.getSystemPasteboard();
        systemPasteboard.setData(pasteData, (err, data) => {
          if (err) {
            hilog.info(0x0000, 'mainTag', 'Failed to set pasteboard: %{public}s', JSON.stringify(err));
            return;
          } else {
            promptAction.showToast({
              message: "Copied to pasteboard",
              duration: 1000,
              bottom: "center",
            })
          }
        });
      }

      if (testNapi.checkPaste()) {
        // need to paste
        const atManager: abilityAccessCtrl.AtManager = abilityAccessCtrl.createAtManager();
        atManager.requestPermissionsFromUser(getContext(), ['ohos.permission.READ_PASTEBOARD']).then(async () => {
          let systemPasteboard: pasteboard.SystemPasteboard = pasteboard.getSystemPasteboard();
          let data = await systemPasteboard.getData();
          hilog.info(DOMAIN, 'testTag', 'Got pasteboard data: %{public}s', JSON.stringify(data));
          let count = data.getRecordCount();
          for (let i = 0;i < count;i++) {
            let record = data.getRecord(i);
            hilog.info(DOMAIN, 'testTag', 'Got pasteboard record: %{public}s', JSON.stringify(record));
            let plainText: string = record.plainText;

            let textEncoder = util.TextEncoder.create('utf-8');
            let encodeResult = textEncoder.encode(plainText); 

            let base64 = new util.Base64Helper();
            let encoded = base64.encodeToStringSync(encodeResult);
            testNapi.pushPaste(encoded);
          }

          promptAction.showToast({
            message: "Pasted from pasteboard",
            duration: 1000,
            bottom: "center",
          })
        });
      }
    }, 100);
  }

  onPageHide() {
    clearInterval(this.timer);
  }

  @Builder MenuBuilder() {
    Menu() {
      MenuItem({ content: "Paste" })
        .onClick(async () => {
          const atManager: abilityAccessCtrl.AtManager = abilityAccessCtrl.createAtManager();
          await atManager.requestPermissionsFromUser(getContext(), ['ohos.permission.READ_PASTEBOARD']);
          let systemPasteboard: pasteboard.SystemPasteboard = pasteboard.getSystemPasteboard();
          let data = await systemPasteboard.getData();
          hilog.info(DOMAIN, 'testTag', 'Got pasteboard data: %{public}s', JSON.stringify(data));
          let count = data.getRecordCount();
          for (let i = 0;i < count;i++) {
            let record = data.getRecord(i);
            hilog.info(DOMAIN, 'testTag', 'Got pasteboard record: %{public}s', JSON.stringify(record));
            let plainText: string = record.plainText;
            let textEncoder = util.TextEncoder.create('utf-8');
            let encodeResult = textEncoder.encode(plainText); 
            testNapi.send(encodeResult.buffer);
          }
        })
    }
  }

  build() {
    Row() {
      Column() {
        XComponent({
          type: XComponentType.SURFACE,
          controller: this.xComponentController
        })
        .bindContextMenu(this.MenuBuilder, ResponseType.RightClick)
      }
      .width('100%')
    }
    .height('100%')
    .onTouch((event: TouchEvent) => {
      // hilog.info(DOMAIN, 'testTag', 'Got touch: %{public}s', JSON.stringify(event));
      for (let touch of event.touches) {
        if (touch.type === TouchType.Down) {
          this.touchState.set(touch.id, touch.y);
        } else if (touch.type === TouchType.Move) {
          // we use pixels, convert from vp to px
          let offset = vp2px(this.touchState.get(touch.id) as number - touch.y);
          this.touchState.set(touch.id, touch.y);
          // hilog.info(DOMAIN, 'testTag', 'Got touch offset: %{public}d', offset);
          testNapi.scroll(offset);
        }
      }
    })
    .onKeyEvent((event: KeyEvent) => {
      hilog.info(DOMAIN, 'testTag', 'Got key: %{public}s', JSON.stringify(event));
      if (event.type === KeyType.Down) {
        if (this.leftCtrlPressed && event.unicode as number >= 97 && event.unicode as number <= 122) {
          // Ctrl-A to Ctrl-Z
          let buffer = new ArrayBuffer(1);
          let view = new Uint8Array(buffer);
          view[0] = event.unicode as number - 97 + 1; // ^A is 0x1
          testNapi.send(buffer);
        } else if (event.keyText === "KEYCODE_EQUALS") {
          let buffer = new ArrayBuffer(1);
          let view = new Uint8Array(buffer);
          // workaround
          if (event.unicode === 0x3d) {
            view[0] = 0x2b;
          } else {
            view[0] = 0x3d;
          }
          testNapi.send(buffer);
        } else if (event.unicode !== 0) {
          let textEncoder = util.TextEncoder.create('utf-8');
          let encodeResult = textEncoder.encode(String.fromCharCode(event.unicode as number)); 
          testNapi.send(encodeResult.buffer);
        } else {
          if (event.keyText === "KEYCODE_CTRL_LEFT") {
            this.leftCtrlPressed = true;
          } else if (keyMapping.has(event.keyText)) {
            let data: number[] = keyMapping.get(event.keyText)!;
            let buffer = new ArrayBuffer(data.length);
            let view = new Uint8Array(buffer);
            let i = 0;
            for (let byte of data) {
              view[i] = byte;
              i += 1;
            }
            testNapi.send(buffer);
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
