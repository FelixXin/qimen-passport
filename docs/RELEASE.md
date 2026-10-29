<p align="right"><strong>English</strong> · <a href="RELEASE.zh_CN.md">简体中文</a></p>

# Local release preparation

1. Confirm a public app name, contributor account, short descriptions, symbol-summary sharing rights and representative screenshots.
2. Create a real upstream-based fork/feature branch; this extracted folder has no Git history or remote and has not been committed/pushed. Review only intended source, tests, licenses and documentation. Do not copy build/, managed_components/, .agents/, private snapshots or previous mixed-app assets.
3. Run the complete validation gate, inspect native screenshots, record exact full/app/ELF hashes. Do not transfer old mixed-app hardware acceptance to this version.
4. With a separate explicit device/write-impact approval, test this standalone firmware: cold boot without wordbooks, full-date manual setting, calendar/palace/articles, scrolling/paging/back/wake, saved brightness/Wi-Fi, web phone-time readback, correct/incorrect network configuration and BLE window/cleanup. Measure radio memory and power separately if making claims.
5. Prepare a reviewed public source URL, representative cover and verified merged full.bin. Publish to GitHub and the official AI Passport community only after explicit authorization. Validate the actual released artifact, not a later rebuild. Never upload local raw logs or ELF/MAP automatically.
6. Optionally propose a text-only application archive to upstream's current plays/<username>/<app-name>/ convention after publication; confirm the current upstream instructions when doing so. The original hardware-test main is not a place to merge the entire application.

Publishing actions are recorded separately; this checklist itself does not authorize future updates or flashing. Initial app version: 0.1.0. A full installation can replace old settings; older reader layouts need a separate compatible app-only update review.
