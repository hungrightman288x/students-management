# Student Management — Quản lý sinh viên C++17

Ứng dụng console theo mô hình OOP và `Manager<T>`, lưu dữ liệu bằng file TXT. Đã có quản lý sinh viên, giáo viên, môn học, lớp học phần, tài khoản, đăng ký học và điểm; menu phân quyền cho admin, giáo viên và sinh viên.

## Chạy nhanh trên Windows

Máy cần có G++ hỗ trợ C++17 trong PATH. Tại thư mục gốc dự án:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1 -Test
.\build\student_management.exe
```

Script build bằng G++ và chạy bộ kiểm tra C++. Python chỉ cần nếu muốn chạy thêm kiểm thử giao diện console:

```powershell
python tests/cli_smoke.py build/student_management.exe
```

Tài khoản mẫu trong `data/` (chỉ dùng dữ liệu giả để học/demo):

| Tên đăng nhập | Mật khẩu | Vai trò / hồ sơ |
| --- | --- | --- |
| `admin` | `admin123` | Admin |
| `teacher1` | `teacher123` | Giáo viên `T001`, lớp `CS101-01` |
| `teacher2` | `teacher123` | Giáo viên `T002`, lớp `CS102-01` |
| `student1` | `student123` | Sinh viên `S001`, đăng ký `E001` |
| `student2` | `student123` | Sinh viên `S002`, chưa đăng ký |

Luồng demo: admin đăng ký lớp cho sinh viên → đăng xuất → giáo viên nhập điểm bằng mã đăng ký → đăng xuất → sinh viên xem điểm → thoát và mở lại để kiểm tra lưu trữ.

Dùng một thư mục dữ liệu mới nếu muốn bắt đầu từ đầu. Chương trình yêu cầu tạo admin đầu tiên, không tự thêm tài khoản vào dữ liệu đã có:

```powershell
.\build\student_management.exe --data-dir data-local
.\build\student_management.exe --data-dir data-local --check-data
```

`--check-data` đọc và kiểm tra toàn bộ dữ liệu rồi thoát, không mở menu. Đường dẫn dữ liệu mặc định là `data/` tính từ thư mục đang chạy. Nhập số để chọn menu; `0` quay lại, đăng xuất hoặc thoát tùy màn hình. Thao tác sửa giữ nguyên mã và nhập lại thông tin của bản ghi.

## Build bằng CMake

Cần CMake từ 3.16 và trình biên dịch hỗ trợ C++17. Không cần thư viện bên ngoài.

```sh
cmake -S . -B build/cmake
cmake --build build/cmake --config Debug
cmake -E chdir build/cmake ctest -C Debug --output-on-failure
```

Với Visual Studio, chạy `build/cmake/Debug/student_management.exe`. Với Ninja/MinGW hoặc Linux, chạy `build/cmake/student_management` (thêm `.exe` trên Windows). Muốn chọn MinGW trên Windows, cấu hình bằng `cmake -S . -B build/cmake -G "MinGW Makefiles"` trong terminal đã có G++ và mingw32-make.

CMake tạo thư viện `student_core`, ứng dụng `student_management` và bộ kiểm tra `student_management_tests`. CTest tự thêm kiểm thử console nếu tìm thấy Python 3. Cả hai bộ kiểm tra dùng thư mục tạm riêng, không sửa `data/` của dự án.

## Chức năng và phân quyền

| Vai trò | Chức năng |
| --- | --- |
| Admin | Thêm/sửa/xóa/tìm theo mã/hiển thị sinh viên, giáo viên, môn học, lớp học phần và tài khoản; đăng ký/hủy học; sửa ngày đăng ký; xem điểm; đặt lại mật khẩu |
| Giáo viên | Xem hồ sơ, các lớp được phân công và điểm của các lớp đó; nhập/sửa điểm cho sinh viên trong lớp mình phụ trách |
| Sinh viên | Xem hồ sơ, lớp đã đăng ký và điểm của chính mình |
| Mọi tài khoản | Đổi mật khẩu của mình sau khi xác nhận mật khẩu cũ; đăng xuất |

Mã đối tượng không được rỗng hoặc trùng. Tài khoản sinh viên/giáo viên liên kết bằng `ProfileID`; tên đăng nhập không cần trùng mã hồ sơ. Admin không có `ProfileID`. Không xóa hoặc đổi vai trò của admin đang đăng nhập.

Đăng ký yêu cầu sinh viên và lớp tồn tại, chưa đăng ký trùng và lớp còn chỗ. Hủy đăng ký giảm sĩ số. Sửa lớp giữ nguyên số sinh viên, không được giảm sức chứa xuống dưới sĩ số hoặc đổi môn khi đã có sinh viên. Muốn đổi sinh viên/lớp của một đăng ký phải hủy rồi đăng ký mới; sửa ngày đăng ký giữ nguyên điểm.

Chỉ giáo viên đang phụ trách lớp được sửa điểm; admin không nhập điểm thay giáo viên. Điểm phải hữu hạn và nằm trong `[0, 10]`. Công thức tổng kết: `10% chuyên cần + 30% giữa kỳ + 60% cuối kỳ`. Điểm khởi tạo là 0; phiên bản hiện tại chưa phân biệt điểm 0 với chưa nhập điểm.

Khi xóa, hệ thống từ chối nếu đối tượng vẫn được tham chiếu: sinh viên có đăng ký/tài khoản, giáo viên có lớp/tài khoản, môn có lớp, hoặc lớp có đăng ký. Hãy gỡ liên kết trước. Nhập ngày không hợp lệ, sai kiểu số hoặc lựa chọn ngoài menu sẽ được báo lỗi.

## Cấu trúc code

```text
include/, src/
  models/       Account, Student, Teacher, Subject, CourseSection, Enrollment, Grade, Date
  managers/     Manager<T>, PersistentManager<T>, 6 manager cụ thể, SchoolManager
  storage/      Đọc/ghi bản ghi TXT và thay file
  ui/           Menu console
src/main.cpp    Khởi động, tham số dòng lệnh, kiểm tra dữ liệu
scripts/        Script build G++ trên Windows
tests/          Kiểm tra nghiệp vụ/lưu trữ và kiểm thử console
data/           6 file dữ liệu mẫu
docs/           Quy ước làm việc nhóm ban đầu
```

`Manager<T>` sở hữu đối tượng bằng `unique_ptr`, không cho copy manager và đặt định nghĩa template trong header. Với `Add(T*)`, manager nhận quyền sở hữu khi thêm thành công; khi trả về `false`, bên gọi vẫn giữ con trỏ. Có overload `Add(unique_ptr<T>)` để quản lý tự động cả trường hợp thêm thất bại. Danh sách hiển thị sắp theo mã.

`PersistentManager<T>` dùng chung phần đọc/ghi file. Các manager cụ thể dùng lại lớp này. `SchoolManager` phối hợp liên kết giữa các manager, kiểm tra quyền và giữ phiên đăng nhập. Mọi thao tác thay đổi từ UI đi qua `SchoolManager`. `Menu` xử lý nhập liệu/hiển thị, để phần nghiệp vụ có thể dùng lại khi làm GUI sau này.

## Định dạng dữ liệu TXT

UTF-8 (đọc được cả BOM và CRLF của Windows), mỗi bản ghi một dòng. Trường chuỗi đặt trong dấu ngoặc kép, phân tách các trường bằng khoảng trắng. Bên trong chuỗi, escape `"` thành `\"` và `\` thành `\\` theo `std::quoted`. Dòng trống và dòng bắt đầu bằng `#` được bỏ qua. Không dùng ký tự xuống dòng/tab bên trong các trường chuỗi.

| File | Thứ tự trường |
| --- | --- |
| `accounts.txt` | `"username" "password" role "profileID"` |
| `students.txt`, `teachers.txt` | `"id" "fullName" day month year "email"` |
| `subjects.txt` | `"id" "name" credits` |
| `course_sections.txt` | `"id" "subjectID" "teacherID" maxStudents studentNumber` |
| `enrollments.txt` | `"id" "studentID" "sectionID" day month year attendance midterm final` |

`role`: `0 = STUDENT`, `1 = TEACHER`, `2 = ADMIN`. Email có thể rỗng; tên, mã, mật khẩu và các liên kết bắt buộc phải hợp lệ. Ngày trong khoảng năm 1–9999 và tuân thủ năm nhuận. Tổng điểm được tính lại từ ba thành phần; không lưu riêng. Sĩ số trong file lớp phải đúng bằng số đăng ký tương ứng.

Dữ liệu lỗi dừng khởi động và báo file/dòng khi có thể, không bỏ qua bản ghi lỗi và không ghi đè dữ liệu đó. Các file chưa tồn tại được xem là danh sách rỗng nhưng các liên kết giữa danh sách vẫn phải hợp lệ.

## Lưu và phục hồi

Menu tự lưu sau mỗi thay đổi thành công, có thêm mục Lưu cho admin và lưu lại khi thoát nếu còn thay đổi. Lỗi ghi được báo rõ; thay đổi còn trong bộ nhớ để thử lưu lại. Không lưu trong destructor.

Việc lưu toàn bộ hệ thống tạo dữ liệu mới và sao lưu dữ liệu cũ trong `.student-management-save` ở thư mục dữ liệu, sau đó thay các file. Nếu bị ngắt trước khi hoàn tất, lần mở tiếp theo phục hồi bản cũ trước khi đọc dữ liệu. Không tự xóa thư mục phục hồi khi chương trình chưa xử lý xong. Chỉ chạy một tiến trình trên cùng một thư mục dữ liệu; chưa hỗ trợ nhiều tiến trình cùng ghi hoặc đồng bộ cloud.

Mật khẩu hiện lưu trực tiếp trong TXT theo mô hình bài tập; ô nhập console hiện hiển thị ký tự khi gõ. Chỉ sử dụng tài khoản và dữ liệu giả cho bản demo này.

Quy ước chia việc và thao tác Git ban đầu nằm trong [docs/TEAM_WORKFLOW.md](docs/TEAM_WORKFLOW.md).
