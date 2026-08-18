# `ci/build.yml` — 需要手动应用

`ci/build.yml` 是**修复后的 CI 工作流**，但它没能直接提交到
`.github/workflows/build.yml`：本次改动由 GitHub App 令牌推送，
而该令牌没有 `workflows` 权限，GitHub 会拒绝这样的 push：

```
refusing to allow a GitHub App to create or update workflow
`.github/workflows/build.yml` without `workflows` permission
```

## 必须手动执行这一步

**在合并本 PR 后，仓库里仍然是旧的、构建不出蓝牙的工作流。**
请由有权限的账号执行：

```bash
git checkout arena/01a013b2-ble-fixture
cp ci/build.yml .github/workflows/build.yml
git rm -r --cached ci && rm -rf ci      # 可选：应用后删除本目录
git commit -am "Apply fixed BLE fixture CI workflow"
git push
```

## 为什么这一步不能省

旧工作流构建的是 **ZMK 自己的 app**，且用 `-b nice_nano`（不带 `//zmk` 变体），
导致 `CONFIG_BT=n` —— 产物里**完全没有蓝牙协议栈**，所以扫描不到设备。

而且本 PR 已经删掉了旧工作流依赖的 `config/nice_nano.keymap`，
所以在应用新工作流之前，旧工作流会直接构建失败。
只有替换掉工作流，CI 才会开始构建本仓库的 `app/`，
并断言 `CONFIG_BT=y`、广播名、以及 `0x26000` 链接地址。
