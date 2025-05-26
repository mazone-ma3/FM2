// 曲データMML サンプル
// 文字配列変数にMMLを記述してpsgcomp.cをコンパイルして実行する
// 書式はMSX-BASIC準拠
// Pで再生パート終了
// !でダミーパート終了
// (複数の曲を記述可能。PSG/SSG.comの第2パラメータで指定)
// ,ENDまたは*でコンパイル終了
// ノイズ出力設定はSOUND文またはPSG/SSG.comの第3パラメータを使ってください。
// (YコマンドでY7,28のようにも出来ますが機種によって上位2bitの値が変わります。)

unsigned char *data[] = {
// 1曲目
	"t150o5v12c1<a4.f4a8>c4<b4.g4.e4.d2.r8c1<a4.f4>c8e4d4.g4.e4.d4<b2r8>>c1<a4.f4a8>c4<b4.>g4.e4.d4<b4.g4c1<a4.f4>c8e4d4.g4.e4.d4<b2r8",
	"P",	// パート1終了
	"t150o2v10l8ff>c<br8afaff>c<br8afagg>dc<r8babgg>dc<r8babff>c<br8afaff>c<br8afagg>dc<r8babgg>dc<r8babff>c<br8afaff>c<br8afagg>dc<r8babgg>dc<r8babff>c<br8afaff>c<br8afagg>dc<r8babgg>dc<r8bab",
	"P",	// パート2終了
	"!",	// 使わないパート

// 2曲目
	"V13T180L8O6GEGEAGFEGEGEFEDCO6GEGEAGFEGEGEFEDCO6DO5GO6DO5GO6EDCO5GABO6CO5AGAGEO6DO5GO6DO5GO6EDCO5GO6DEFAG2",
	"P",	// パート1終了
	"O3V13T180L8CGCGCACACGCGO2GBGBO3CGCGCACACGCGO2BO3GO2BO3GO2GBGBO3CECEO2GBGBO3CECEO2GBGBO3CECEO3FEDCO2BAGF",
	"P",	// パート2終了
	"T180S8M3000L4CCCCCCCCM3000CCCCCCCCM3000CCM300CCM3000CCM300CCM3000CCM300CCR8CCM3000L8CC",
	"P",	// パート3終了

	"END"	// コンパイル終了
};

