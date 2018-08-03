#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "junqi.h"
#include "board.h"

void select_flag_event(GtkWidget *widget, GdkEventButton *event, gpointer data);

int OsRead(int fd, void *zBuf, int iAmt, long iOfst)
{
  off_t ofst;
  int nRead;

  ofst = lseek(fd, iOfst, SEEK_SET);
  if( ofst!=iOfst ){
    return 0;
  }
  nRead = read(fd, zBuf, iAmt);

  return nRead;
}

void LoadChess(Junqi *pJunqi, enum ChessColor color)
{
	int iWidth, iHeight;
	GdkPixbuf *pColor;
	int i;

	iWidth = 36;
	iHeight = 27;
	pColor = pJunqi->ChessImage[color];

	pJunqi->Chess[color][NONE] = NULL;
	pJunqi->Chess[color][DARK] = gdk_pixbuf_new_subpixbuf(pColor,0,0,iWidth,iHeight);
	pJunqi->Chess[color][JUNQI] = gdk_pixbuf_new_subpixbuf(pColor,9*iWidth,0,iWidth,iHeight);
	pJunqi->Chess[color][DILEI] = gdk_pixbuf_new_subpixbuf(pColor,11*iWidth,0,iWidth,iHeight);
	pJunqi->Chess[color][ZHADAN] = gdk_pixbuf_new_subpixbuf(pColor,12*iWidth,0,iWidth,iHeight);
	pJunqi->Chess[color][SILING] = gdk_pixbuf_new_subpixbuf(pColor,10*iWidth,0,iWidth,iHeight);
	for(i=JUNZH; i<=GONGB; i++)
	{
		pJunqi->Chess[color][i] = gdk_pixbuf_new_subpixbuf(pColor,(i-5)*iWidth,0,iWidth,iHeight);
	}
}

void LoadChessImage(Junqi *pJunqi)
{
    int i=0;
	pJunqi->ChessImage[ORANGE] = gdk_pixbuf_new_from_file("./res/orange.bmp",NULL);
	pJunqi->ChessImage[PURPLE] = gdk_pixbuf_new_from_file("./res/purple.bmp",NULL);
	pJunqi->ChessImage[GREEN] = gdk_pixbuf_new_from_file("./res/green.bmp",NULL);
	pJunqi->ChessImage[BLUE] = gdk_pixbuf_new_from_file("./res/blue.bmp",NULL);
	for(i=0; i<4; i++)
	{
		LoadChess(pJunqi,i);
	}

}

void InitLineup(Junqi *pJunqi, enum ChessColor color)
{
	int fd;
	u8 aBuf[4096];
	Jql *pLineup;
	int i;
	//fd = open("./res/5.jql", O_RDWR|O_CREAT, 0600);
	fd = open("D:/军旗/高手/5.jql", O_RDWR|O_CREAT, 0600);

	OsRead(fd, aBuf, 4096, 0);
	pLineup = (Jql*)(&aBuf[0]);
	if( memcmp(pLineup->aMagic, aMagic, 4)!=0 )
	{
		assert(0);
	}
	for(i=0; i<30; i++)
	{
		pJunqi->Lineup[color][i].type = pLineup->chess[i];
	}

}

void SetChessImageType(Junqi *pJunqi, int dir, int i, int iType)
{
	GdkPixbuf *pPixbuf;
	GdkPixbuf *pRotate;
	GtkWidget *pImage;
    if(dir==RIGHT||dir==LEFT)
    {
    	iType = DARK;
    }
	pPixbuf = pJunqi->Chess[(dir+2)%4][iType];
	pImage = gtk_image_new_from_pixbuf(pPixbuf);
	pJunqi->Lineup[dir][i].pImage[0] = pImage;
	pJunqi->Lineup[dir][i].pImage[2] = pImage;
	pRotate = gdk_pixbuf_rotate_simple(pPixbuf,GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE);
	//g_object_unref (pPixbuf);//这里pPixbuf是某个图片的子图片，释放掉会出错
	pPixbuf = pRotate;
	pImage = gtk_image_new_from_pixbuf(pPixbuf);
	pJunqi->Lineup[dir][i].pImage[1] = pImage;
	pRotate = gdk_pixbuf_rotate_simple(pPixbuf,GDK_PIXBUF_ROTATE_UPSIDEDOWN);
	g_object_unref (pPixbuf);
	pPixbuf = pRotate;
	pImage = gtk_image_new_from_pixbuf(pPixbuf);
	pJunqi->Lineup[dir][i].pImage[3] = pImage;
	g_object_unref (pPixbuf);
}

void SetChess(Junqi *pJunqi, enum ChessDir dir)
{
	enum ChessType iType;
	GtkWidget *pImage;

	int x,y;
	int i;

	for(i=0;i<30;i++)
	{
		iType = pJunqi->Lineup[dir][i].type;

		assert( iType>=NONE && iType<=GONGB );

		switch(dir)
		{
		case HOME:
			x = 265+(i%5)*40;
			y = 13+(i/5+11)*39;
			break;
		case RIGHT:
			x = 38+(i/5+11)*39;
			y = 242+(4-i%5)*39;
			break;
		case OPPS:
			x = 265+(4-i%5)*40;
			y = 13+(5-i/5)*39;
			break;
		case LEFT:
			x = 38+(5-i/5)*39;
			y = 242+(i%5)*39;
			break;
		default:
			assert(0);
			break;
		}
		pJunqi->ChessPos[dir][i].xPos = x;
		pJunqi->ChessPos[dir][i].yPos = y;
		pJunqi->ChessPos[dir][i].pLineup = &pJunqi->Lineup[dir][i];
		pJunqi->ChessPos[dir][i].type = pJunqi->Lineup[dir][i].type;
		if(iType!=NONE)
		{
			SetChessImageType(pJunqi, dir, i, iType);
			for(int j=1; j<4; j++)
			{
				pImage = pJunqi->Lineup[dir][i].pImage[j];
				gtk_fixed_put(GTK_FIXED(pJunqi->fixed), pImage, x,y);
			}
			pImage = pJunqi->Lineup[dir][i].pImage[dir];
			gtk_widget_show(pImage);
		}
	}
}


BoardChess *GetChessPos(Junqi *pJunqi, int x, int y)
{
	BoardChess *pChess = NULL;
	int iDir;
	int iPosX;
	int iPosY;
	int iPos;

	//这里数字的最大值不能乱改，因为是向下取整，改大了会触发断言
	if( (x>=MIDX) && (x<= MIDX+HORIZONTAL_LONG1)  )
	{
		if( (y>=BOTTOMY) && (y<=BOTTOMY+VERTICAL_LONG) )
		{
			iDir = HOME;
			iPosX = (x-MIDX)/LENGTH2;
			iPosY = (y-BOTTOMY)/LENGTH1;
			assert(iPosX>=0 && iPosX<=4);
			assert(iPosY>=0 && iPosY<=5);
			if( (x<=MIDX+iPosX*LENGTH2+36) && (y<=BOTTOMY+iPosY*LENGTH1+27) )
			{
				iPos = iPosY*5 + iPosX;
				pChess = &pJunqi->ChessPos[iDir][iPos];
			}
		}
		else if( (y>=TOPY) && (y<=TOPY+VERTICAL_LONG) )
		{
			iDir = OPPS;
			iPosX = (x-MIDX)/LENGTH2;
			iPosY = (y-TOPY)/LENGTH1;
			assert(iPosX>=0 && iPosX<=4);
			assert(iPosY>=0 && iPosY<=5);
			if( (x<=MIDX+iPosX*LENGTH2+36) && (y<=TOPY+iPosY*LENGTH1+27) )
			{
				iPos = (5-iPosY)*5 + 4-iPosX;
				pChess = &pJunqi->ChessPos[iDir][iPos];
			}
		}
		else if( (y>=NINE_GRID_Y) && (y<=NINE_GRID_Y+HORIZONTAL_LONG2) )
		{
			iDir = HOME;
			iPosX = (x-NINE_GRID_X)/(2*LENGTH2);
			iPosY = (y-NINE_GRID_Y)/(2*LENGTH1);
			assert(iPosX>=0 && iPosX<=4);
			assert(iPosY>=0 && iPosY<=4);
			if( (x<=MIDX+iPosX*2*LENGTH2+36) && (y<=NINE_GRID_Y+iPosY*2*LENGTH1+27) )
			{
				iPos = iPosY*3 + iPosX;
				pChess = &pJunqi->NineGrid[iPos];
			}
		}
	}
	else if( (y>=MIDY) && (y<=MIDY+HORIZONTAL_LONG2)  )
	{
		if((x>=RIGHTX) && (x<= RIGHTX+VERTICAL_LONG))
		{
			iDir = RIGHT;
			iPosX = (x-RIGHTX)/LENGTH1;
			iPosY = (y-MIDY)/LENGTH1;
			assert(iPosX>=0 && iPosX<=5);
			assert(iPosY>=0 && iPosY<=4);
			if( (x<=RIGHTX+iPosX*LENGTH1+27) && (y<=MIDY+iPosY*LENGTH1+36) )
			{
				iPos = iPosX*5 + 4-iPosY;
				pChess = &pJunqi->ChessPos[iDir][iPos];
			}
		}
		else if( (x>=LEFTX) && (x<= LEFTX+VERTICAL_LONG) )
		{
			iDir = LEFT;
			iPosX = (x-LEFTX)/LENGTH1;
			iPosY = (y-MIDY)/LENGTH1;
			assert(iPosX>=0 && iPosX<=5);
			assert(iPosY>=0 && iPosY<=4);
			if( (x<=LEFTX+iPosX*LENGTH1+27) && (y<=MIDY+iPosY*LENGTH1+36) )
			{
				iPos = (5-iPosX)*5 + iPosY;
				pChess = &pJunqi->ChessPos[iDir][iPos];
			}
		}
	}
	if( pChess )
	{
		pChess->iDir = iDir;
	}

	return pChess;
}

void HideChess(BoardChess *pChess, int iDir)
{
	if(pChess->type!=NONE)
	{
		gtk_widget_hide(pChess->pLineup->pImage[iDir]);
	}
}

void ShowRectangle(Junqi *pJunqi, BoardChess *pChess, int color)
{
	GtkWidget *image;
	int x,y;
	//上家和下家，用竖框
	if(pChess->iDir&1)
	{
		x = pChess->xPos-4;
		y = pChess->yPos-10;
	}
	else
	{
		x = pChess->xPos-4;
		y = pChess->yPos-4;
	}
	if( color==RECTANGLE_RED )
	{
		image = pJunqi->redRectangle[pChess->iDir&1];
	}
	else
	{
		image = pJunqi->whileRectangle[pChess->iDir&1];
	}
	gtk_fixed_move(GTK_FIXED(pJunqi->fixed),
			                 image,
		                     x,y);
	gtk_widget_show(image);
}

void MoveFlag(Junqi *pJunqi, BoardChess *pChess, int isInit)
{
	int flagX,flagY;
	if(pChess->iDir&1)
	{
		flagX = pChess->xPos+3;
		flagY = pChess->yPos+7;
	}
	else
	{
		flagX = pChess->xPos+7;
		flagY = pChess->yPos+2;
	}

	if(isInit)
	{
		gtk_fixed_move(GTK_FIXED(pJunqi->fixed),pChess->pLineup->pFlag,flagX,flagY);
	}
	else
	{
		gtk_fixed_put(GTK_FIXED(pJunqi->fixed),pChess->pLineup->pFlag,flagX,flagY);
	}
	gtk_widget_show(pChess->pLineup->pFlag);
}

void MoveChess(Junqi *pJunqi, BoardChess *pChess)
{
	BoardChess *pSrc, *pDst;


	if(!pJunqi->bSelect)
	{
		if( pChess->type==NONE )
		{
			return;
		}
		pJunqi->bSelect = 1;
		pJunqi->pSelect = pChess;

		//显示白色选择框
		gtk_widget_hide(pJunqi->redRectangle[0]);
		gtk_widget_hide(pJunqi->redRectangle[1]);

		ShowRectangle(pJunqi, pChess, RECTANGLE_WHITE);
	}
	else
	{
		gtk_widget_hide(pJunqi->whileRectangle[0]);
		gtk_widget_hide(pJunqi->whileRectangle[1]);
		if(pChess==pJunqi->pSelect)
		{
			pJunqi->bSelect = 0;
			pJunqi->pSelect = NULL;
			return;
		}
		pJunqi->bSelect = 0;
		pDst = pChess;
		HideChess(pDst,pChess->iDir);
		pSrc = pJunqi->pSelect;
		pDst->pLineup = pSrc->pLineup;

		gtk_fixed_move(GTK_FIXED(pJunqi->fixed),
				       pSrc->pLineup->pImage[pDst->iDir],
				       pDst->xPos,pDst->yPos);

		if(pDst->pLineup->pFlag)
		{
			MoveFlag(pJunqi,pDst,1);
		}

		//注意！这部分代码有很强的顺序关系
		pDst->type = pSrc->type;
		//隐藏选中的棋子
		HideChess(pSrc,pSrc->iDir);
		pSrc->type = NONE;

		gtk_widget_show(pSrc->pLineup->pImage[pDst->iDir]);

		ShowRectangle(pJunqi, pChess, RECTANGLE_RED);
	}
}

/*
 * 重新初始化标记图片
 * gtk的fixed有一个非常不友好的特性，后加入的控件会遮挡之前的控件
 * 目前没发现有什么方法把被遮挡的控件显示在最前面，所以只有
 * 先把被遮挡的控件销毁，再重新放到fixed里面
 */
void InitFlagImage(Junqi *pJunqi)
{
	GtkWidget *FlagImage = gtk_event_box_new();
	GtkWidget *image;
	char *imageName = "./res/flag1.png";
	gtk_fixed_put(GTK_FIXED(pJunqi->fixed), FlagImage, 0, 0);
	g_signal_connect(FlagImage, "button-press-event",
			G_CALLBACK(select_flag_event), pJunqi);
	image = gtk_image_new_from_file(imageName);
	gtk_container_add(GTK_CONTAINER(FlagImage),image);
	gtk_widget_show(image);
	pJunqi->flagObj.image = FlagImage;
}

/*
 * 右键点击时显示标记图片，不同的区域显示的相对位置有所不同
 */
void ShowFlagChess(Junqi *pJunqi, BoardChess *pChess)
{
	int x,y;
	switch(pChess->iDir)
	{
	case HOME:
	case LEFT:
		x = pChess->xPos+40;
		y = pChess->yPos-250;
		break;
	case OPPS:
		x = pChess->xPos+40;
		y = pChess->yPos+40;
		break;
	case RIGHT:
		x = pChess->xPos-195;
		y = pChess->yPos-250;
		break;
	default:
		break;
	}
	gtk_widget_destroy(pJunqi->flagObj.image);
	InitFlagImage(pJunqi);

	gtk_fixed_move(GTK_FIXED(pJunqi->fixed),
			       pJunqi->flagObj.image,
			       x, y);
	gtk_widget_show(pJunqi->flagObj.image);
	pJunqi->pSelect = pChess;
}

/*
 * 当鼠标点击棋盘时会触发该事件，移动棋子、标棋等
 */
void deal_mouse_press(GtkWidget *widget, GdkEventButton *event, gpointer data)
{

	int x,y;
	BoardChess *pChess;

	Junqi *pJunqi = (Junqi *)data;
	x = event->x;
	y = event->y;

	pChess=GetChessPos(pJunqi,x,y);
	gtk_widget_hide(pJunqi->flagObj.image);
	if( pChess==NULL )
	{
		return;
	}
	//点击左键
    if( event->button==1 )
    {
    	if(pJunqi->bStart)
    	{
    		MoveChess(pJunqi, pChess);
    	}
    }
    else if( event->button==3 )
    {
    	//点击右键
    	if(pChess->type!=NONE)
    	{
    		ShowFlagChess(pJunqi, pChess);
    	}
    }
}

/*
 * 选择标记棋子，点击标记图片后会触发该事件
 */
void select_flag_event(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	int x,y;
	int iPosX;
	int iPosY;
	int iPos;
	GdkPixbuf *pixbuf;
	Junqi *pJunqi = (Junqi *)data;
	x = event->x;
	y = event->y;

	gtk_widget_hide(pJunqi->flagObj.image);
	if( x>=15 && x<=181)
	{
		if( y>=15 && y<=210 )
		{
			iPosX = (x-15)/44;
			if(y<=92)
			{
				iPosY = (y-15)/40;
			}
			else
			{
				iPosY = (y-18)/40;
			}
			if( (x<=15+iPosX*44+34) && (y<=15+iPosY*40+34) )
			{
				iPos = iPosX+iPosY*4;
				pixbuf = pJunqi->flagObj.paPixbuf[iPos];
				if(pJunqi->pSelect->pLineup->pFlag)
				{
					gtk_widget_destroy(pJunqi->pSelect->pLineup->pFlag);
				}
				pJunqi->pSelect->pLineup->pFlag = gtk_image_new_from_pixbuf(pixbuf);
				MoveFlag(pJunqi,pJunqi->pSelect,0);

			}
		}
		else if( (x>=49 && x<=146) && (y>=221 && y<=253) )
		{
			if(pJunqi->pSelect->pLineup->pFlag)
			{
				gtk_widget_destroy(pJunqi->pSelect->pLineup->pFlag);
				pJunqi->pSelect->pLineup->pFlag = NULL;
			}
		}
	}
}

void InitSelectImage(Junqi *pJunqi)
{
	pJunqi->whileRectangle[0] = GetSelectImage(0, 0);
	pJunqi->whileRectangle[1] = GetSelectImage(1, 0);
	pJunqi->redRectangle[0] = GetSelectImage(0, 1);
	pJunqi->redRectangle[1] = GetSelectImage(1, 1);
	gtk_fixed_put(GTK_FIXED(pJunqi->fixed), pJunqi->whileRectangle[0], 0, 0);
	gtk_fixed_put(GTK_FIXED(pJunqi->fixed), pJunqi->whileRectangle[1], 0, 0);
	gtk_fixed_put(GTK_FIXED(pJunqi->fixed), pJunqi->redRectangle[0], 0, 0);
	gtk_fixed_put(GTK_FIXED(pJunqi->fixed), pJunqi->redRectangle[1], 0, 0);

}

/*
 * 初始化标记棋子
 */
void InitFlagPixbuf(Junqi *pJunqi)
{
	GdkPixbuf *pixbuf;
	GdkPixbuf *temp;
	int x,y;
	char *imageName = "./res/flag1.png";
	int i;

	InitFlagImage(pJunqi);

	pixbuf = gdk_pixbuf_new_from_file(imageName, NULL);
	for(i=0; i<20; i++)
	{
		x = 15+(i%4)*43;
		if(i<8)
		{
			y = 16+(i/4)*38;
		}
		else
		{
			y = 25+(i/4)*37;
		}
		temp = gdk_pixbuf_new_subpixbuf(pixbuf,x,y,34,34);
		temp = gdk_pixbuf_scale_simple(temp,22,22,GDK_INTERP_BILINEAR);
		pJunqi->flagObj.paPixbuf[i] = temp;
	}
}

void InitBoardItem(Junqi *pJunqi)
{
	//初始化选择框
	InitSelectImage(pJunqi);
	//初始化标记棋子
	InitFlagPixbuf(pJunqi);
}

void InitNineGrid(Junqi *pJunqi)
{
	int i;
	for(i=0; i<9; i++)
	{
		pJunqi->NineGrid[i].xPos = NINE_GRID_X+(i%3)*2*LENGTH2;
		pJunqi->NineGrid[i].yPos = NINE_GRID_Y+(i/3)*2*LENGTH1;
		pJunqi->NineGrid[i].type = NONE;
		pJunqi->NineGrid[i].iDir = HOME;

	}

}

/*
 * 初始化棋盘、棋子和标记棋等相关控件
 */
void CreatBoardChess(GtkWidget *window, Junqi *pJunqi)
{
	g_signal_connect(gtk_widget_get_parent(pJunqi->fixed), "button-press-event",
			G_CALLBACK(deal_mouse_press), pJunqi);

	LoadChessImage(pJunqi);
	for(int i=0; i<4; i++)
	{
		InitLineup(pJunqi,i);
		SetChess(pJunqi,i);
	}
	InitNineGrid(pJunqi);
	InitBoardItem(pJunqi);
}

Junqi *JunqiOpen(void)
{
	Junqi *pJunqi = (Junqi*)malloc(sizeof(Junqi));
	memset(pJunqi, 0, sizeof(Junqi));

	return pJunqi;
}

void ConvertFilename(char *zName)
{
	while(*zName!='\0')
	{
		if(*zName=='\\')
		{
			*zName = '/';
		}
		zName++;
	}
}

void SwapChess(Junqi *pJunqi, int i, int j)
{
	ChessLineup *temp;
	BoardChess *pChess;
	int dir = pJunqi->selectDir;

	temp = pJunqi->ChessPos[dir][j].pLineup;
	pJunqi->ChessPos[dir][j].pLineup = pJunqi->ChessPos[dir][i].pLineup;
	pJunqi->ChessPos[dir][i].pLineup = temp;
	pJunqi->ChessPos[dir][i].type = pJunqi->ChessPos[dir][i].pLineup->type;
	pJunqi->ChessPos[dir][j].type = pJunqi->ChessPos[dir][j].pLineup->type;

	pChess = &pJunqi->ChessPos[dir][i];
	gtk_fixed_move(GTK_FIXED(pJunqi->fixed),
				   pChess->pLineup->pImage[pChess->iDir],
				   pChess->xPos,pChess->yPos);

	pChess = &pJunqi->ChessPos[dir][j];
	gtk_fixed_move(GTK_FIXED(pJunqi->fixed),
				   pChess->pLineup->pImage[pChess->iDir],
				   pChess->xPos,pChess->yPos);
}

void select_chess_cb (GtkNativeDialog *dialog,
                  gint             response_id,
                  gpointer         user_data)
{
	char *name;
	Junqi *pJunqi = (Junqi *)user_data;
	GtkFileChooserNative *native = pJunqi->native;
	int fd;
	u8 aBuf[4096];
	Jql *pLineup;
	int i,j;

	if (response_id == GTK_RESPONSE_ACCEPT)
	{
		name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (native));
		ConvertFilename(name);

		fd = open(name, O_RDWR|O_CREAT, 0600);
		OsRead(fd, aBuf, 4096, 0);
		pLineup = (Jql*)(&aBuf[0]);
		if( memcmp(pLineup->aMagic, aMagic, 4)!=0 )
		{
			assert(0);
		}

		for(i=0;i<30;i++)
		{
			if(pJunqi->ChessPos[0][i].type == NONE)
			{
				continue;
			}
			if( pJunqi->ChessPos[0][i].type != pLineup->chess[i] )
			{
				for(j=i+1;j<30;j++)
				{
					if(pLineup->chess[i]==pJunqi->ChessPos[0][j].type)
					{
						SwapChess(pJunqi,i,j);
						break;
					}
				}
			}
		}
	}

	gtk_native_dialog_destroy (GTK_NATIVE_DIALOG (native));
	g_object_unref (native);

}