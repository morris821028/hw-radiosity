# Parallel Computing and Optimization Skills for Radiosity

R04922067 楊翔雲 R04922133 古君葳

國立台灣大學 資訊工程學研究所

---

## Introduction

熱輻射算法公式 

`\(B_i dA_i = E_i dA_i + R_i \int_j B_j F_{ji} dA_j\)`

假設平面上有 `\(N\)` 個三角形，每一次迭代最多需要 `\(O(N^2)\)`。如果判定兩點之間是否能傳遞只需要 `\(O(\log N)\)`，則會落入 `\(O(N \log N)\)`

---

### Form-Factor Computation ###

其中佔據效能因素是 Form-Factor 估算，因此有像 Hemicube 之類的近似逼近，大幅度減少計算量，但投影回到原本物件上會失真。

`\(F_{ij} = \frac{1}{A_i} \int_{A_i}\int_{A_j} \frac{\cos \phi_2 \cos \phi_1}{\pi r^2} dA_i dA_j\)`

.center[![](http://i.imgur.com/mOMEPZR.gif)]

---
## Optimization

### Improve I-cache Miss ###

壓縮程式碼長度以改善 I-cache miss，因為大部分的初始化只會運行一次，不應該交錯在時常被呼叫的函數之間，指令載入效能才會提高，同時也要做好 Code Layout，就能改善執行效能。

#### Old ####

```c
x, y = compute(0)
buildTree(x, y)
x, y = compute(1)
buildTree()
x, y = compute(2)
buildTree()
x, y = compute(3)
buildTree()
...
```

#### New ####

```c
for i from 0 to n
    x, y = compute(i)
    buildTree() 
```

---

### Improve Data Locality ###

程式碼中使用 3D-DDA Line Algorithm/Bresenham's Line Algorithm 搭配 Octree，在找尋某個射線 `\(\vec{v} = p + t \cdot d\)` 與哪個最近三角形相交。

* 只需要儲存葉節點代表的立方體中，所有可能相交的三角形編號
* 移除掉中間產生的編號，讓每一次 access 的 cache-miss 下降


然而，在 3D-DDA 中，我們需要反查找空間中一點 `\(p\)` 在哪一個葉節點中，藉由固定的長寬高切割長度，可以在 `\(O(1)\)` 時間內得知 `[i][j][k]` 各別的值。

* 若限制大小，則建立 `[i][j][k]` 查找
* 若自適應大小，則建立 hash 表查找
* 根據實驗結果，效能並沒有改善，因為三角形個數過多導致命中機率過低。

----

> 對於 Static Tree 的 Memory Layout，大致上分成 
>	* DFS Layout
>	* Inorder Layout
>	* BFS Layout 
>	* van Emde Boas Layout
>
> 目前程式使用的是 DFS Layout

---

### Short Circuit Design ###

判斷三角形與一個正交立方體是否相交，使用投影到二維空間中與一線段的交點是否皆存在。投影方法有 3 種 x-y, y-z, z-x，線段投影共有 2 種，共計 6 種情況。

* 原先程式沒有做好短路設計，只要其中一種不符就應退出


#### Old ####
```cpp
int CrossOver(TrianglePtr tri, 
			Vector g0, Vector g1) {
	for (xyz = 0; xyz < 3; xyz++) {
		// front face project
		// ...
		if (!test())
			return false;
		// back face project
		// ...
		if (!test())
			return false;
	}
	return true;
}
```

![](http://i.imgur.com/8jcycpG.png)


---

### Clipping Algorithm ###

我們實作課程中所描述的 Cohen–Sutherland Algorithm 降低 branch 次數，使用 bitwise 操作引出 SSE (Streaming SIMD Extensions)。

* 儘管 compiler `-O2` 替我們優化，為減少 stack push/pop 的次數，實作時請不要使用的 procedure call，否則會慢上許多。

```c
int Clip(Vector p0, Vector p1, Vector g0, Vector g1, int x, int y) {
	char mask1 = (p0[x] < g0[x])<<0 |
		(p0[x] > g1[x])<<1 |
		(p0[y] < g0[y])<<2 |
		(p0[y] > g1[y])<<3 ;
	char mask2 = (p1[x] < g0[x])<<0 |
		(p1[x] > g1[x])<<1 |
		(p1[y] < g0[y])<<2 |
		(p1[y] > g1[y])<<3 ;
	if (mask1&mask2)	
		return false;
	if (!(mask1|mask2))
	return true;
	// ... test
}
```

---

### Strength Reduction for Float-Point ###

兩個外積結果相乘小於零，減少 instruction cycle 量，盡量用整數作為運算型態。


#### Old ####
```c
float a = cross(/* */);
float b = cross(/* */);
if (a * b < 0)
	return false;
b = cross(/* */);
if (a * b < 0)
	return false;
...
```

#### New ####

```c
int a = cross(/* */) < 0;
int b = cross(/* */) < 0;
if (a != b)
	return false;
b = cross(/* */) < 0;
if (a != b)
	return false;
...
```
]


---

### Strength Reduction for Ray Casting ###


判斷 ray 是否能打到三角形 `\(A\)` 上，先用 bounding box 包住 `\(A\)`，計算 `\(p\)` 到 bounding box 的時間 `\(t\)`，若 `\(t\)` 大於目前的最小 `\(t_{\min}\)`，則退出。相反地，再計算更精準的 `\(t\)`。


加入利用已知結果 `\(\vec{v} = p + t_{\text{min}} \cdot d, \;\; t_{\text{min}} > 0\)`


```c
int TriangleHitted(Vector p, Vector d, TrianglePtr tp, float *t_min) {
	float t = /* time t from p to bounding box of Triangle tp */;
	if (t < eps)
		return false;
	if (t >= *t_min)	/* important !! */
		return false;
	/* ... */
	*t_min = t;
	return true;
}
```

---

### Shrink the Scope of Variables ###

減少變數生命週期的長度以增加放入暫存器的機會，而非 stack 上。

#### Old ####
```c
float rgb[3];
for (int i = 0; i < 3; i++)
	rgb[f(i)] = g(i);
/* ... */
if (maybe) {
	for (int i = 0; i < 3; i++) {
		rgb[h(i)] = g(i);
	}
	/* ... */
}
```

#### New ####
```c
{
	float rgb[3];
	for (int i = 0; i < 3; i++)
		rgb[f(i)] = g(i);
	/* ... */
}
if (maybe) {
	float rgb[3];
	for (int i = 0; i < 3; i++) {
		rgb[h(i)] = g(i);
	}
	/* ... */
}
```

---

### Reduce the Number of Arguments ###

減少 stack push/pop 次數

> 特別在平行處理上，編譯器不敢針對可能是 share 的變數進行 Copy Optimization

#### Old ####
```c
struct Arg {
	int a0, a1;
};	// p1.a1 = p2.a0
int f(Arg p1, Arg p2) {
	/* ... */
}
```

#### New ####

```c
struct Arg {
	int a0, a1, a2;
};
int f(Arg p1p2) {
	/* ... */
}

```

---

### Remove Implications of Pointer Aliasing - Problem ###

移除指標 Aliasing，意指可能會指向相同記憶體位址，導致每次計算都要重新載入，不能放進暫存器中。

如下述的寫法，編譯器無法判定 `srcTri` 是否與 `desTri` 相同，在累加時則重新載入 `srcTri->deltaB[]` 的數值，計算上可能會產生數次的 cache miss，隨著迴圈次數不斷突顯效能差異。

```cpp
void computeRadiosity(TrianglePtr srcTri, TrianglePtr desTri, float ff[3]) {
	for (int v = 0; v < 3; v++) {	// vertex
		for (int c = 0; c < 3; c++) {	// color RGB
			flaot deltaB = desTri->Frgb[c]/255.0*RefRatio*srcTri->deltaB[c]*ff[v]/3;
			desTri->deltaB[c] += deltaB; 
			desTri->accB[v][c] += deltaB; 
			desTri->deltaAccB[v][c] += deltaB; 
		}
	}
}
```

> 解法請參考下一頁

---

### Remove Implications of Pointer Aliasing - Solution ###

* __方法 1__: 加入 `if (srcTri != desTri)` 判斷，讓編譯器在 Function Pass 階段著手的 Dependency Analysis 更好
* __方法 2__: 使用 Copy Optimization，同時把重複計算搬到 stack 上，或者使用 Polyhedal 表示法進行 Reordering Accesses for Loop Nest。這裡我們選擇前者，更容易引出 SSE。

```cpp
void computeRadiosity(TrianglePtr srcTri, TrianglePtr desTri, float ff[3]) {
	const float k = RefRatio / 255.0;
	float lo[3] = { desTri->Frgb[0]*k*(srctri->deltaB[0]), 
					desTri->Frgb[1]*k*(srctri->deltaB[1]), 
					desTri->Frgb[2]*k*(srctri->deltaB[2])};
	for (int v = 0; v < 3; v++)	{	// vertex
		for (int c = 0; c < 3; c++) {	// color RGB
			/* calculate the reflectiveness */
			float deltaB = lo[c] * ff[v] / 3;
			desTri->deltaB[c] += deltaB;
			desTri->accB[v][c] += deltaB;
			desTri->deltaaccB[v][c] = deltaB;
		}
	}
}
```

> 改善 `\(10\%\)` 效能

---

### Copy Optimization ###

我們可以考慮使用 Copy 的方式放入較近的 stack 上，而非傳遞指標

> 將資料放得近可提升 cache 使用率，隨著使用次數增加而明顯

> 當運行次數多，才能讓 data reused 的比例上升，意即 `\(n\)` 要夠大。

#### Old ####
```c
int f(const Triangle *from) {
	for (int i = 0; i < n; i++)	
		compute(from, i);
}
```
#### New ####
```c
int f(const Triangle *from) {
	Triangle tmp = *from;
	for (int i = 0; i < n; i++)	
		compute(tmp, i);
}

```


---

### Strength Reduction for Form-Factor ###

根據公式 `\(F_{ij} \approx \int_{y \in A_j} \frac{\cos \theta_x \cos \theta_y}{\pi r^2} V(x, y) dA_y\)` 計算。

```c
float computeFormFactor(TrianglePtr srcTri, int logSrc, TrianglePtr desTri, int logDes, Vector p) {
	Vector dir = srcTri->c - p;
	float ff = 0;
	if (RayHitted(p, dir, logDes) == logDes) {
		float theta1 = CosTheta(dir, srcTri->normal), theta2 = CosTheta(dir, desTri->normal);
		ff = theta1 * theta2 * srcTri->area / (norm2(dir) * PI);
	}
	return max(ff, 0.f);
}
```

效能考量的因素：

* `RayHitted` 需要大量的計算  

* `\(\frac{\cos \theta_x \cos \theta_y}{\pi r^2}\)` 在 `float` 儲存格式下可能無法得到貢獻

> 解法如下頁所示

---

### Strength Reduction for Form-Factor - Solution ###

優先計算 Form-Factor 的值，若存在貢獻再運行 `RayHitted` 判斷。


```c
float computeFormFactor(TrianglePtr srcTri, int logSrc, TrianglePtr desTri, int logDes, Vector p) {
	Vector dir = srcTri->c - p;
	float theta1 = CosTheta(dir, srcTri->normal), theta2 = CosTheta(dir, desTri->normal);
	float ff = theta1 * theta2;
	if (ff <= 0)	return 0.f;
	ff *= srcTri->area / (norm2(dir) * PI);
	if (ff <= 0)	return 0.f;
	if (RayHitted(p, dir, logDes) == logDes)
		return ff;
	return 0.f;
}
```

> 改善近 `\(200\%\)` 的效能

---


### Comment

* 使用洽當的編譯器參數可加速 2 倍
* 減少 Form-Factor 計算加速 2 倍
* 剩餘優化部份改善 10% ~ 20% 效能。

至今，我們加速了 .big[.center[4 倍]]

### Experiment


| Model           | Origin (sec.)        | Our v0.1 (sec.) | Speedup |
|:---------------:|---------------------:|-------------------:|--------:|
| room.tri        | 10.27                | 4.70               | 2.18    |
| hall.tri        | 176.92               | 38.50              | 4.59    |
| church.tri      | 72.32                | 42.64              | 1.69    |


---

### Snapshot - Blocks ###


![Parallel 9 sec. blocks](http://i.imgur.com/SZYs2sq.png)

---

### Snapshot - Room ###


![Parallel 60 sec. room](http://i.imgur.com/fv11dZu.png)

---

### Snapshot - Hall ###


![Parallel 60 sec. hall](http://i.imgur.com/k5D4wHn.png)

---

### Snapshot - Church ###


![Parallel 60 sec. church](http://i.imgur.com/oZtcYqd.png)

---

## Parallel Algorithm

### Review Serial Algorithm

原始的算法中，每一次會挑選單位面積能量最多的那塊三角形 `\(f\)`，針對盤面上所有三角形 `\(t\)` 進行輻射。在輻射的過程中，若 `\(t\)` 三個頂點之間的能量差異大，可選擇自適應切割三角形。


```c
while (not converage) {
	f = MaxRadiosityTriangle()
	foreach triangle t in model
		shader(f, t);
	clearRadiosity(f)
}
```

---

### Review Adaptive Splitting Method

當偵測到綠色三角形 `\(A\)` 頂點之間的 Form-Factor 差異過大時，使用最長邊中點切割，盡量產生銳角三角形，並且針對鄰居三角形分裂。

為減少計算量，只算新增中心點 `\(P\)` 的 Form-Factor。對於下方的三角形 `\( B \)` 而言，分成兩種情況，已在這一輪完成計算，則重新計算 Form-Factor；反之，不做任何事。

![Adaptive Triangle Splitting](http://i.imgur.com/qd2QlKk.png)

---

### Parallel Design

每一次挑選亮度前 `\(20\%\)` 三角形作為光源，將其他三角形嘗試與其進行熱輻射

> 可以只選擇最亮的，這麼做是為了提早看到近似最終結果的場景

```c
while (not converage) {
	set<Triangle> f = RadiosityTriangleCandiateCandidate();
	parallel foreach triangle t in model
		if (f.find(t))
			continue;
		foreach s in f
			shader(s, t);
	clearRadiosity(f);
}
```

> 平行無法搭配自適應切割

Q: 為什麼按照中線切割無法自適應？  
A: 因為鄰居不在同一個 thread 中，那麼在切割鄰居就無法傳遞正確的能量。

---

### Splitting Triangle in Parallel - Center

為了解決鄰居在不同 thread 的問題，可以使用三角形的重心進行切割，可以解決 Dependency 的問題。

![](http://i.imgur.com/cI7z6Jx.png)

---

### Prepartition with Splitting

一開始就根據重心切好三角形，之後不進行自適應。下圖皆以過程中最多產生 30000 個三角形為限制。

| Longest Edge | Center      |
|:------------:|:-----------:|
| ![](http://i.imgur.com/0pUng1j.png) | ![](http://i.imgur.com/ZyLbimO.png) |

明顯地，根據重心的切割方法容易產生鈍角三角形，看起來就會像很多紡錘體。

---

### Compare room.tri  ###

Processor `\(P = 6\)` on HP-z420 Server

| Parallel           | Serial v0.1    |
|:------------------:|:--------------:|
| 18.89 sec.         | 27.64 sec.     |
| ![](http://i.imgur.com/iVU7hKW.png) | ![](http://i.imgur.com/nikXJI5.png) |
 
---

### Compare hall.tri  ###

Processor `\(P = 6\)` on HP-z420 Server

| Parallel           | Serial v0.1    |
|:------------------:|:--------------:|
| 27.44 sec.         | 35.83 sec.     |
| ![](http://i.imgur.com/8fHEPmy.png) | ![](http://i.imgur.com/C7YmRQr.png) |
 
---


## Conclusion

* 透過編譯器技術，我們重新改寫助教的程式碼，並且加速 4 倍之多  

* 基於多核心平台，我們提出平行計算的方法，並探討一些平行上的困難與解決方案，最後在 HP-z420 Server 上減少 `\(30\%\)` 的時間，就可得到近似的結果。

----
