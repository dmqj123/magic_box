import 'components.dart';

List<ResultItemCard> getResultItems(String query) {
  //TEST
  List<ResultItemCard> result_list = [];
  if (query != "") {
    result_list.add(
      ResultItemCard(
        content: "问Ai",
        imageUrl:
            "https://is1-ssl.mzstatic.com/image/thumb/Purple221/v4/6d/dd/58/6ddd5814-706a-9075-4e1e-00b65633a307/AppIcon-0-0-1x_U007epad-0-11-0-85-220.png/492x0w.webp",
      ),
    );
  }
  if (query.length > 2) {
    result_list.add(
      ResultItemCard(
        content: "搜索-Bing",
        imageUrl:
            "https://ts1.tc.mm.bing.net/th/id/OIP-C.wscQIcgzAsAL1AX2T4OCwQAAAA?rs=1&pid=ImgDetMain&o=7&rm=3",
      ),
    );
  }
  if (query.contains("ro")) {
    result_list.add(
      ResultItemCard(
        content: "C:\\Users\\abcdef\\Downloads\\robot.png",
        title: "robot.png",
        imageUrl:
            "https://bpic.588ku.com/element_origin_min_pic/19/04/23/fce1a95fe111509e2717f2f02435a60b.jpg",
        preview_path: "https://kooly.faistudio.top/material/icon.png",

      ),
    );
  }

  return result_list;
}
