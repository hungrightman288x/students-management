# Quy ước làm việc nhóm

Phân công và quy trình Git ban đầu của dự án, giữ lại để nhóm tham khảo.

## 7. Phân công cho hai thành viên và tên branch

Nhóm sử dụng một repository chung với **ba branch chính: `main` và hai branch làm việc**. Chưa có tên hai thành viên nên tạm dùng Thành viên 1 và Thành viên 2; nhóm tự gán người cho từng vai trò.

| Branch | Người phụ trách | Phạm vi |
| --- | --- | --- |
| `main` | Cả nhóm review | Bộ khung và code đã được ghép, kiểm tra |
| `feat/academic-management` | Thành viên 1 | Sinh viên, giáo viên, môn học, lớp học phần và các thành phần nền tảng |
| `feat/account-enrollment-ui` | Thành viên 2 | Tài khoản, đăng nhập, phân quyền, đăng ký học, điểm và tích hợp menu |

### 7.1. Thành viên 1 — `feat/academic-management`

**Chức năng:**

- Quản lý sinh viên, giáo viên, môn học và lớp học phần: thêm, sửa, xóa, tìm kiếm và hiển thị.
- Gán giáo viên phụ trách cho lớp học phần; quản lý sức chứa của lớp.
- Viết các hàm đọc/ghi dữ liệu cho phần mình phụ trách.
- Triển khai `Manager<T>` và `Date` theo thiết kế hai người đã thống nhất.
- Cung cấp hàm kiểm tra sinh viên, lớp học phần và giáo viên phụ trách để Thành viên 2 sử dụng.

**File phụ trách:**

| Nhóm file | Danh sách |
| --- | --- |
| `include/models/` và `src/models/` | `Student.h/.cpp`, `Teacher.h/.cpp`, `Subject.h/.cpp`, `CourseSection.h/.cpp`, `Date.h/.cpp` |
| `include/managers/` và `src/managers/` | `StudentManager.h/.cpp`, `TeacherManager.h/.cpp`, `SubjectManager.h/.cpp`, `CourseSectionManager.h/.cpp` |
| Thành phần template | `include/managers/Manager.h` — đặt cả khai báo và định nghĩa ở đây |
| Dữ liệu mẫu | `data/students.txt`, `data/teachers.txt`, `data/subjects.txt`, `data/course_sections.txt` |

Ký hiệu `Student.h/.cpp` nghĩa là hai file: `include/models/Student.h` và `src/models/Student.cpp`; các cặp còn lại áp dụng tương tự.

### 7.2. Thành viên 2 — `feat/account-enrollment-ui`

**Chức năng:**

- Quản lý tài khoản, đăng nhập, đăng xuất và điều hướng theo vai trò.
- Đăng ký học, kiểm tra trùng đăng ký và kiểm tra sức chứa lớp bằng dữ liệu Thành viên 1 cung cấp.
- Nhập, cập nhật và tính điểm; kiểm tra giáo viên phụ trách trước khi cho sửa điểm.
- Cho sinh viên xem các lớp đã đăng ký và kết quả học tập của mình.
- Đọc/ghi tài khoản, đăng ký học và điểm.
- Viết menu, ghép các chức năng và duy trì cấu hình build chung.

**File phụ trách:**

| Nhóm file | Danh sách |
| --- | --- |
| `include/models/` và `src/models/` | `Account.h/.cpp`, `Enrollment.h/.cpp`, `Grade.h/.cpp` |
| `include/managers/` và `src/managers/` | `AccountManager.h/.cpp`, `EnrollmentManager.h/.cpp` |
| Giao diện | `include/ui/Menu.h`, `src/ui/Menu.cpp` |
| Tích hợp | `src/main.cpp`, `CMakeLists.txt` |
| Dữ liệu mẫu | `data/accounts.txt`, `data/enrollments.txt` — điểm lưu cùng đăng ký học theo định dạng đã chốt |

### 7.3. Thứ tự phối hợp

1. **Cùng chốt thiết kế:** tên hàm trong `.h`, tham số, kiểu trả về, định dạng TXT, quy tắc phân quyền và công thức tính điểm.
2. **Chuẩn bị nền tảng:** Thành viên 1 ưu tiên `Manager.h`, `Date` và các header cần dùng chung; Thành viên 2 chuẩn bị `main.cpp` tối thiểu và CMake. Đưa từng phần lên PR để ghép sớm.
3. **Phát triển song song:** mỗi người viết cả `.h`, `.cpp` và đọc/ghi dữ liệu thuộc phần mình. Thành viên 2 có thể viết phần tài khoản trong khi chờ các chức năng học vụ.
4. **Tích hợp:** Thành viên 1 gửi tên hàm và cách gọi chức năng cho Thành viên 2 để nối vào `Menu`. Hạn chế cả hai cùng sửa `Menu.cpp` và `CMakeLists.txt`.
5. **Kiểm tra chung:** đăng nhập → đăng ký lớp → giáo viên nhập điểm → sinh viên xem điểm → đóng và mở lại chương trình để kiểm tra dữ liệu.

Khi Thành viên 1 thêm file `.cpp`, ghi rõ danh sách trong PR để Thành viên 2 cập nhật CMake trước khi kiểm tra tích hợp. Nếu cần tự sửa file do người khác phụ trách, trao đổi trước để tránh sửa chồng nhau.


## 9. Quy trình làm việc nhóm với GitHub

### 9.1. Quy ước branch

- `main`: chỉ nhận bộ khung ban đầu và code đã được review, tích hợp.
- Thành viên 1 làm trên `feat/academic-management`.
- Thành viên 2 làm trên `feat/account-enrollment-ui`.
- Cả hai dùng chung repository; mỗi người clone về máy riêng.
- Với cách chia cố định này, giữ lại hai branch sau khi merge PR để tiếp tục phần việc sau. Không xóa branch hoặc tạo lại bằng `git switch -c` mỗi ngày.
- Tên branch là quy ước làm việc, không tự hạn chế quyền sửa file của từng người.

### 9.2. Tạo branch — mỗi người chỉ làm một lần

**Thành viên 1**, chạy tại thư mục gốc dự án:

```bash
git switch main
git pull --ff-only origin main
git switch -c feat/academic-management
git push -u origin feat/academic-management
```

**Thành viên 2**, chạy tại thư mục gốc dự án trên máy của mình:

```bash
git switch main
git pull --ff-only origin main
git switch -c feat/account-enrollment-ui
git push -u origin feat/account-enrollment-ui
```

Các lệnh trên giả định branch chưa được tạo. Nếu branch đã có trên máy, dùng `git switch TEN_BRANCH`. Nếu chỉ có trên GitHub, chạy `git fetch origin` rồi `git switch --track origin/TEN_BRANCH`, thay `TEN_BRANCH` bằng tên đã phân công.

### 9.3. Tiếp tục làm việc và cập nhật code chung

Trước khi chuyển branch hoặc cập nhật, commit phần việc hiện tại; không chuyển sang branch khác khi còn thay đổi chưa xử lý.

Mỗi người chuyển về branch của mình:

```bash
# Thành viên 1
git switch feat/academic-management
```

```bash
# Thành viên 2
git switch feat/account-enrollment-ui
```

Sau đó cập nhật code đã được ghép vào `main`:

```bash
git fetch origin
git merge origin/main
```

Nếu có conflict, kiểm tra và sửa các đoạn xung đột, lưu file, `git add` các file đã giải quyết rồi `git commit` để hoàn tất merge. Không chọn bỏ toàn bộ code của một bên khi chưa hiểu phần khác nhau.

### 9.4. Commit và push phần việc

**Thành viên 1:**

```bash
git status
git add .
git commit -m "feat(academic): add student management"
git push -u origin feat/academic-management
```

**Thành viên 2:**

```bash
git status
git add .
git commit -m "feat(account): add login and role checks"
git push -u origin feat/account-enrollment-ui
```

Đổi nội dung commit cho đúng thay đổi thực tế. Sau khi thiết lập theo dõi, có thể dùng `git push`. Chỉ commit khi có thay đổi; không cần chờ làm xong toàn bộ nhiệm vụ mới commit.

### 9.5. Tạo Pull Request và ghép code

| Người tạo PR | Base (nhánh nhận code) | Compare (nhánh chứa thay đổi) | Người review |
| --- | --- | --- | --- |
| Thành viên 1 | `main` | `feat/academic-management` | Thành viên 2 |
| Thành viên 2 | `main` | `feat/account-enrollment-ui` | Thành viên 1 |

1. Push phần việc lên branch của mình rồi tạo PR theo bảng trên.
2. Ghi chức năng đã làm, cách kiểm tra, file CMake cần bổ sung và phần còn thiếu nếu có.
3. Người còn lại review, kiểm tra phần liên quan và yêu cầu sửa nếu cần. Push thêm commit lên cùng branch sẽ cập nhật PR đang mở.
4. Chỉ ghép khi người review đồng ý và phần code trong PR đã được kiểm tra phù hợp. Các PR chỉ cập nhật tài liệu hoặc bộ khung chưa chạy được cần ghi rõ giới hạn đó.
5. Với hai branch dùng lại lâu dài, chọn **Create a merge commit** khi merge PR, nếu repository cho phép; không xóa branch sau khi merge.
6. Sau khi PR được merge, cả hai cập nhật branch của mình bằng `git fetch origin` và `git merge origin/main` trước khi tiếp tục. Đây là bước cần thiết để branch vừa merge cũng nhận phần tích hợp mới trên `main`.

Khi tiếp tục sửa sau một PR đã merge, commit và push lên branch được giao rồi tạo **PR mới**. PR cũ đã merge không mở lại để bổ sung code. Nếu cần xem bản chung trên máy, chuyển sang `main` và chạy `git pull --ff-only origin main` khi thư mục làm việc đã sạch.

Không force push lên `main` để xử lý lỗi hoặc xung đột.

