# Kế hoạch hoàn thiện quản lý điểm sinh viên

Cập nhật ngày 28/10/2026. Buổi làm tiếp theo: 29/10/2026.

Mục tiêu ưu tiên trong thời gian một tuần: đăng nhập, sinh viên xem điểm của mình, giáo viên nhập/sửa điểm lớp mình phụ trách, lưu và đọc lại điểm. Dùng dữ liệu TXT mẫu để tạo sẵn hồ sơ, lớp và đăng ký học. Chức năng tạo tài khoản và menu admin đầy đủ triển khai sau.

## Phần đã có

- [x] Các model `Student`, `Teacher`, `Subject`, `CourseSection`, `Enrollment`, `Grade`, `Date`, `Account`, `Role`.
- [x] Kiểm tra định dạng ID của sinh viên, giáo viên, môn, lớp học phần và đăng ký.
- [x] `Grade` kiểm tra khoảng điểm thông thường và tính tổng kết theo 10%–30%–60%.
- [x] `Manager<T>` quản lý map `ID -> T*`, có thêm, xóa, cập nhật, tìm và hiển thị.
- [x] `AccountManager` đọc `accounts.txt`, tạo tài khoản và thêm vào map.
- [x] `AccountManager::Login()` tìm username, kiểm tra mật khẩu, trả về tài khoản hoặc `nullptr`.
- [x] `main()` thử đăng nhập bằng dữ liệu cố định và kiểm tra `nullptr` trước khi hiển thị.

## 1. Bắt đầu ngày mai: StudentManager

Làm trọn phần này trước khi chuyển sang manager khác.

- [ ] Trong `include/managers/StudentManager.h`, cho lớp kế thừa `Manager<Student>` và include các header cần thiết.
- [ ] Thêm thuộc tính đường dẫn file, constructor nhận đường dẫn và hàm `LoadFromFile()`, theo cách đã làm với `AccountManager`.
- [ ] Trong `src/managers/StudentManager.cpp`, mở file, đọc từng dòng và tách đủ các trường.
- [ ] Chuyển ngày, tháng, năm từ chuỗi sang số nguyên; từ chối chuỗi số sai định dạng hoặc có dữ liệu thừa.
- [ ] Tạo `Student` với ID, tên, ngày sinh và email; gọi `Add()` được kế thừa.
- [ ] Nếu `Add()` từ chối đối tượng mới vì trùng ID, giải phóng đối tượng đó và báo lỗi.
- [ ] Báo lỗi khi không mở/đọc được file hoặc dữ liệu không hợp lệ; bỏ qua dòng trống và xử lý ký tự `\r` cuối dòng nếu có.
- [ ] Thêm vài sinh viên mẫu vào `data/students.txt`.
- [ ] Thử tạo một `StudentManager`, gọi `PrintAll()` và `FindByID()` với mã có/không có trong danh sách.

Định dạng đề xuất:

```text
StudentID|FullName|Day|Month|Year|Email
SV001|Nguyen Van An|20|1|2005|an@example.com
```

Dòng đầu trong ví dụ là mô tả các cột, không ghi thành dòng dữ liệu nếu chưa viết xử lý header.

Mốc hoàn thành: đọc được nhiều sinh viên, hiển thị đúng, tìm đúng mã, từ chối ID trùng, ngày sai và dòng thiếu trường.

## 2. TeacherManager và SubjectManager

- [ ] Viết `TeacherManager : public Manager<Teacher>` trong cặp header/source tương ứng.
- [ ] Nạp `data/teachers.txt` theo cách tương tự sinh viên; chuyển và kiểm tra ngày sinh.
- [ ] Viết `SubjectManager : public Manager<Subject>` trong cặp header/source tương ứng.
- [ ] Nạp `data/subjects.txt`; chuyển số tín chỉ sang số nguyên và để model kiểm tra giá trị.
- [ ] Chuẩn bị dữ liệu mẫu, thử hiển thị và tìm ID cho cả hai manager.
- [ ] Kiểm tra file không tồn tại, dòng sai định dạng và ID trùng.

Định dạng đề xuất:

```text
teachers.txt: TeacherID|FullName|Day|Month|Year|Email
subjects.txt: SubjectID|SubjectName|Credits
```

Mốc hoàn thành: có đủ ba danh sách sinh viên, giáo viên, môn học trong RAM để tra cứu.

## 3. CourseSectionManager

- [ ] Viết `CourseSectionManager : public Manager<CourseSection>`.
- [ ] Nạp `data/course_sections.txt`, chuyển sức chứa và sĩ số sang số nguyên, tạo model và thêm vào map.
- [ ] Kiểm tra `SubjectId` tồn tại trong `SubjectManager` và `TeacherId` tồn tại trong `TeacherManager` trước khi chấp nhận dữ liệu lớp.
- [ ] Chọn cách phối hợp các manager: truyền tham chiếu vào hàm xử lý hoặc kiểm tra các liên kết sau khi nạp đủ dữ liệu.
- [ ] Chuẩn bị ít nhất hai lớp thuộc hai giáo viên khác nhau để thử quyền sửa điểm về sau.
- [ ] Thử tìm lớp, lấy mã môn, mã giáo viên và hiển thị sức chứa/sĩ số.
- [ ] Kiểm tra mã lớp trùng, môn/giáo viên không tồn tại, sĩ số âm hoặc vượt sức chứa.

Định dạng đề xuất:

```text
CourseSectionID|SubjectId|TeacherId|MaxStudents|StudentNumber
LHP001|MH001|GV001|40|0
```

Mốc hoàn thành: lớp liên kết đúng với môn và giáo viên có thật.

## 4. EnrollmentManager và dữ liệu điểm

- [ ] Viết `EnrollmentManager : public Manager<Enrollment>`.
- [ ] Nạp `data/enrollments.txt`; chuyển ngày đăng ký và ba thành phần điểm sang kiểu số phù hợp.
- [ ] Tạo `Enrollment` rồi thêm vào map; xử lý dòng lỗi và mã đăng ký trùng.
- [ ] Kiểm tra `StudentID` tồn tại trong `StudentManager` và `CourseSectionID` tồn tại trong `CourseSectionManager`.
- [ ] Chống hai đăng ký khác mã nhưng có cùng cặp `StudentID + CourseSectionID`.
- [ ] Bảo đảm số đăng ký không vượt sức chứa lớp.
- [ ] Thống nhất sĩ số với các đăng ký thực tế; nếu tính lại khi nạp thì không cộng thêm lần nữa vào sĩ số đã lưu trong file lớp.
- [ ] Chuẩn bị dữ liệu: một sinh viên học nhiều lớp, một lớp có nhiều sinh viên và các bộ điểm khác nhau.

Định dạng đề xuất:

```text
EnrollmentID|StudentID|CourseSectionID|Day|Month|Year|Attendance|Midterm|Final
DK001|SV001|LHP001|28|10|2026|8|7|9
```

Điểm tổng kết lấy từ `Grade::GetTotal()`, không cần lưu lặp lại trong file.

Mốc hoàn thành: tra cứu được đăng ký và đi từ đăng ký đến đúng sinh viên, lớp, môn, giáo viên.

## 5. Nối đăng nhập với hồ sơ và xem điểm

- [ ] Sau khi nạp các danh sách, kiểm tra tài khoản role `STUDENT` có hồ sơ sinh viên cùng mã, role `TEACHER` có hồ sơ giáo viên cùng mã. Tài khoản `admin` không bắt buộc có hồ sơ hai loại này.
- [ ] Thay username/mật khẩu viết cố định trong `main()` bằng nhập từ bàn phím. Dùng cách nhập cho phép mật khẩu có khoảng trắng nếu vẫn hỗ trợ dạng đó.
- [ ] Tạo mỗi manager một lần cho phiên chạy; truyền tham chiếu khi chức năng khác cần dùng, tránh sao chép manager chứa con trỏ.
- [ ] Giữ con trỏ tài khoản đăng nhập thành công để lấy username và role; luôn xử lý `nullptr` khi thất bại.
- [ ] Viết chức năng sinh viên xem điểm: lấy StudentID từ tài khoản đã đăng nhập, tìm các Enrollment của chính mã đó.
- [ ] Hiển thị mã/tên môn, mã lớp, điểm thành phần và tổng kết; xử lý trường hợp chưa có đăng ký.
- [ ] Nếu xem một đăng ký cụ thể, kiểm tra StudentID của đăng ký khớp người đăng nhập.

Mốc hoàn thành: sinh viên chỉ xem được dữ liệu học tập của mình.

## 6. Giáo viên nhập/sửa điểm và lưu lại

- [ ] Hiển thị các lớp có `TeacherId` bằng username của giáo viên đang đăng nhập.
- [ ] Cho giáo viên chọn lớp và xem danh sách sinh viên/đăng ký trong lớp đó.
- [ ] Trong chính hàm cập nhật điểm, kiểm tra role là `TEACHER`, tìm đăng ký và lớp, đối chiếu giáo viên phụ trách trước khi sửa.
- [ ] Lấy TeacherID từ tài khoản đã xác thực; không lấy một mã giáo viên bất kỳ do người dùng nhập để cấp quyền.
- [ ] Nhập và kiểm tra toàn bộ điểm cần thay đổi trước khi cập nhật; điểm phải hữu hạn và trong khoảng 0–10.
- [ ] Cập nhật `Grade`, để lớp này tính lại tổng kết, rồi hiển thị kết quả.
- [ ] Viết `EnrollmentManager::SaveToFile()` để lưu toàn bộ đăng ký và điểm hiện tại theo định dạng đã chốt.
- [ ] Sau thay đổi điểm thành công, gọi lưu; báo lỗi nếu mở/ghi file thất bại và giữ dữ liệu trong RAM để có thể thử lưu lại.
- [ ] Tắt chương trình, chạy lại và xác nhận điểm sửa đã được đọc lại đúng.

Mốc hoàn thành: giáo viên chỉ sửa được điểm lớp mình và thay đổi tồn tại qua các lần chạy.

## 7. Menu và cấu hình chạy

- [ ] Viết `Menu` trong `include/ui/Menu.h` và `src/ui/Menu.cpp`, giữ một hàm `main()` duy nhất ở `src/main.cpp`.
- [ ] Menu đăng nhập: nhận thông tin, gọi `Login()`, hiển thị kết quả.
- [ ] Menu sinh viên: xem hồ sơ/điểm của mình và đăng xuất.
- [ ] Menu giáo viên: xem lớp phụ trách, xem bảng điểm, nhập/sửa điểm và đăng xuất.
- [ ] Đăng xuất bằng cách bỏ trạng thái tài khoản hiện tại; không `delete` tài khoản mà manager vẫn sở hữu.
- [ ] Lặp menu đến khi chọn thoát, xử lý lựa chọn và dữ liệu nhập sai.
- [ ] Thêm trực tiếp `<stdexcept>` vào các source đang dùng exception nhưng chưa include: `AccountManager.cpp`, `Student.cpp`, `Teacher.cpp`, `Enrollment.cpp`.
- [ ] Cập nhật `CMakeLists.txt`: hiện mới có `main.cpp` và `Date.cpp`; bổ sung các model, `Manager.cpp`, các manager và source menu đã triển khai.
- [ ] Vì `Manager.cpp` khởi tạo template cho tất cả model, bảo đảm các định nghĩa model tương ứng được liên kết vào chương trình.
- [ ] Chạy chương trình từ thư mục gốc để đường dẫn `data/...` đúng; cập nhật README với lệnh build và chạy thực tế.
- [ ] Bỏ dòng in mật khẩu phục vụ debug trong `Account::DisplayInfo()` khi hoàn thành bản demo.

Mốc hoàn thành: chạy từ đầu đến cuối bằng menu, không phải sửa hằng số trong code cho mỗi lần thử.

## 8. Kiểm tra cuối trước khi nộp/demo

- [ ] Đăng nhập đúng, sai mật khẩu, username không tồn tại; không truy cập con trỏ `nullptr`.
- [ ] Sinh viên có nhiều lớp nhận đúng bộ điểm của từng lớp; không xem được điểm người khác.
- [ ] Giáo viên sửa được lớp mình; bị từ chối khi chọn lớp/đăng ký thuộc giáo viên khác.
- [ ] Điểm tổng kết đúng công thức, từ chối điểm ngoài khoảng và dữ liệu nhập không phải số.
- [ ] Đóng/mở lại vẫn giữ đúng điểm đã sửa.
- [ ] File thiếu, dòng lỗi, ID trùng hoặc ID liên kết không tồn tại đều có thông báo rõ ràng.
- [ ] Sĩ số khớp số đăng ký và không vượt sức chứa.
- [ ] Thử một luồng demo: đăng nhập giáo viên -> sửa điểm -> đăng xuất -> đăng nhập sinh viên -> xem điểm mới -> mở lại chương trình.
- [ ] Kiểm tra `git status`, không đưa file thực thi/build hoặc dữ liệu thật vào commit.

## Các chức năng đã ghi nhận, triển khai sau luồng chính

- [ ] Tạo tài khoản trong `AccountManager` (đã thống nhất để sau, ghi chú ngày 28/10/2026).
  - Nhận username, mật khẩu và role; áp dụng kiểm tra của `Account` và chống trùng username.
  - Với role `STUDENT` hoặc `TEACHER`, username dùng mã hồ sơ và phải có hồ sơ tương ứng tồn tại.
  - Thêm tài khoản vào map và ghi lại `data/accounts.txt` để đăng nhập được sau khi mở lại chương trình.
  - Nếu tạo hồ sơ kèm tài khoản, nhập mã một lần và dùng cho cả hai đối tượng; xử lý nhất quán nếu một bước thất bại.
- [ ] Đổi mật khẩu có kiểm tra mật khẩu cũ, gọi `Account::ChangePassword()` và lưu lại TXT.
- [ ] Thiết kế hàm ghi dữ liệu tài khoản để lưu được các trường riêng tư cần thiết, không dùng `DisplayInfo()` làm định dạng lưu file.
- [ ] Menu admin thêm/sửa/xóa hồ sơ, môn, lớp và tài khoản; thao tác chung dùng lại từ `Manager<T>`.
- [ ] Bổ sung getter ngày sinh hoặc hàm xuất dữ liệu để lưu được đầy đủ Student/Teacher khi có chức năng cập nhật hồ sơ.
- [ ] Đăng ký/hủy học qua menu: kiểm tra trùng, sức chứa, cập nhật sĩ số và lưu dữ liệu liên quan.
- [ ] Quy định xóa dữ liệu có liên kết: ban đầu có thể từ chối xóa sinh viên, giáo viên, môn hoặc lớp đang được tham chiếu.
- [ ] Ngăn sao chép mặc định `Manager<T>` đang sở hữu con trỏ, hoặc chuyển sang cách quản lý sở hữu phù hợp để tránh hai manager cùng xóa một đối tượng.
- [ ] Phân biệt điểm chưa nhập với điểm 0 nếu phạm vi bài yêu cầu; bổ sung học kỳ/năm học cho lớp nếu đề bài yêu cầu.

Chỉ đánh dấu một việc hoàn thành sau khi đã chạy thử kết quả tương ứng. Buổi tiếp theo ưu tiên mục 1; hoàn thành rồi mới chuyển sang mục 2.
