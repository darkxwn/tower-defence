#!/usr/bin/python
import tkinter as tk
from tkinter import messagebox, filedialog
from PIL import Image, ImageTk
import os

class LevelEditor:
    def __init__(self, root):
        self.root = root
        self.root.title("Tower Defence Visual Editor")

        self.width = 12
        self.height = 8
        self.tile_size = 64  # Увеличим для красоты, раз теперь есть спрайты
        self.current_tile_type = 2

        self.level_name = "New Level"
        self.money = 250
        self.tiles = [[0 for _ in range(self.width)] for _ in range(self.height)]
        self.waves = []

        # Пути к спрайтам (подставь свои если папки называются иначе)
        self.sprite_paths = {
            0: "assets/sprites/tile-background .png",
            1: "assets/sprites/tile-portal.png",
            2: "assets/sprites/tile-road.png",
            3: "assets/sprites/tile-base.png",
            4: "assets/sprites/tile-platform.png"
        }

        # Резервные цвета (если спрайт не найден)
        self.colors = {0: "#2c3e50", 1: "#9b59b6", 2: "#ecf0f1", 3: "#e74c3c", 4: "#27ae60"}

        self.images = {} # Тут будут храниться PhotoImage
        self.load_sprites()
        self.setup_ui()

    def load_sprites(self):
        """Загружает и масштабирует спрайты под размер сетки"""
        for tid, path in self.sprite_paths.items():
            if os.path.exists(path):
                img = Image.open(path)
                img = img.resize((self.tile_size, self.tile_size), Image.Resampling.LANCZOS)
                self.images[tid] = ImageTk.PhotoImage(img)
            else:
                print(f"Предупреждение: Файл {path} не найден!")

    def setup_ui(self):
        sidebar = tk.Frame(self.root, padx=10, pady=10)
        sidebar.pack(side=tk.LEFT, fill=tk.Y)

        tk.Label(sidebar, text="Название:").pack()
        self.ent_name = tk.Entry(sidebar)
        self.ent_name.insert(0, self.level_name)
        self.ent_name.pack()

        tk.Label(sidebar, text="Деньги:").pack()
        self.ent_money = tk.Entry(sidebar)
        self.ent_money.insert(0, str(self.money))
        self.ent_money.pack()

        tk.Label(sidebar, text="Размер (W x H):").pack()
        sf = tk.Frame(sidebar); sf.pack()
        self.ent_w = tk.Entry(sf, width=5); self.ent_w.insert(0, str(self.width)); self.ent_w.pack(side=tk.LEFT)
        self.ent_h = tk.Entry(sf, width=5); self.ent_h.insert(0, str(self.height)); self.ent_h.pack(side=tk.LEFT)
        tk.Button(sidebar, text="Изменить размер", command=self.resize_grid).pack(pady=5)

        tk.Label(sidebar, text="--- ИНСТРУМЕНТЫ ---", pady=10).pack()
        self.tool_var = tk.IntVar(value=2)

        # Инструменты с маленькими иконками (если есть)
        tools = [("Пусто", 0), ("Портал (1)", 1), ("Дорога (2)", 2), ("База (3)", 3), ("Платформа (4)", 4)]
        for text, val in tools:
            tk.Radiobutton(sidebar, text=text, variable=self.tool_var, value=val).pack(anchor=tk.W)

        tk.Label(sidebar, text="--- ВОЛНЫ ---", pady=5).pack()
        self.wave_list = tk.Listbox(sidebar, height=6); self.wave_list.pack()
        wc = tk.Frame(sidebar); wc.pack()
        tk.Button(wc, text="+", command=self.add_wave).pack(side=tk.LEFT)
        tk.Button(wc, text="-", command=self.remove_wave).pack(side=tk.LEFT)

        tk.Button(sidebar, text="СОХРАНИТЬ", bg="#27ae60", fg="white", command=self.save_file).pack(side=tk.BOTTOM, fill=tk.X, pady=2)
        tk.Button(sidebar, text="ОТКРЫТЬ", command=self.load_file).pack(side=tk.BOTTOM, fill=tk.X, pady=2)

        self.canvas = tk.Canvas(self.root, bg="#1a1a1a", bd=0, highlightthickness=0)
        self.canvas.pack(side=tk.RIGHT, expand=True, fill=tk.BOTH)
        self.canvas.bind("<Button-1>", self.grid_click)
        self.canvas.bind("<B1-Motion>", self.grid_click)

        self.refresh_canvas()

    def grid_click(self, event):
        x, y = event.x // self.tile_size, event.y // self.tile_size
        if 0 <= x < self.width and 0 <= y < self.height:
            self.tiles[y][x] = self.tool_var.get()
            self.draw_tile(x, y)

    def draw_tile(self, x, y):
        """Отрисовывает конкретный тайл (спрайт или цвет)"""
        val = self.tiles[y][x]
        x1, y1 = x * self.tile_size, y * self.tile_size

        # Удаляем старый объект на этом месте, если он есть
        tag = f"tile_{x}_{y}"
        self.canvas.delete(tag)

        if val in self.images:
            # Рисуем спрайт
            self.canvas.create_image(x1, y1, image=self.images[val], anchor=tk.NW, tags=tag)
        else:
            # Рисуем цветной квадрат (для типа 0 или если нет файла)
            color = self.colors.get(val, "#000000")
            self.canvas.create_rectangle(x1, y1, x1+self.tile_size, y1+self.tile_size,
                                         fill=color, outline="#34495e", tags=tag)

    def refresh_canvas(self):
        self.canvas.delete("all")
        for y in range(self.height):
            for x in range(self.width):
                self.draw_tile(x, y)

        win_w = self.width * self.tile_size + 250
        win_h = max(self.height * self.tile_size, 550)
        self.root.geometry(f"{win_w}x{win_h}")

    def resize_grid(self):
        try:
            nw, nh = int(self.ent_w.get()), int(self.ent_h.get())
            new_tiles = [[0 for _ in range(nw)] for _ in range(nh)]
            for y in range(min(self.height, nh)):
                for x in range(min(self.width, nw)):
                    new_tiles[y][x] = self.tiles[y][x]
            self.width, self.height, self.tiles = nw, nh, new_tiles
            self.refresh_canvas()
        except: messagebox.showerror("Error", "Invalid dimensions")

    def add_wave(self):
        win = tk.Toplevel(self.root)
        win.title("Add Wave")
        tk.Label(win, text="Type (basic/fast/strong):").pack()
        e_t = tk.Entry(win); e_t.insert(0, "basic"); e_t.pack()
        tk.Label(win, text="Count:").pack()
        e_c = tk.Entry(win); e_c.insert(0, "10"); e_c.pack()
        def ok(): self.wave_list.insert(tk.END, f"{e_t.get()}:{e_c.get()}"); win.destroy()
        tk.Button(win, text="OK", command=ok).pack()

    def remove_wave(self):
        sel = self.wave_list.curselection()
        if sel: self.wave_list.delete(sel)

    def save_file(self):
        path = filedialog.asksaveasfilename(defaultextension=".map", filetypes=[("Map files", "*.map")])
        if not path: return
        try:
            with open(path, "w", encoding="utf-8") as f:
                f.write(f"width={self.width}\nheight={self.height}\nmoney={self.ent_money.get()}\nname={self.ent_name.get()}\ntiles=\n")
                for row in self.tiles: f.write(" ".join(map(str, row)) + "\n")
                f.write("waves=\n")
                for i in range(self.wave_list.size()): f.write(self.wave_list.get(i) + "\n")
            messagebox.showinfo("Success", "Saved!")
        except Exception as e: messagebox.showerror("Error", str(e))

    def load_file(self):
        path = filedialog.askopenfilename(filetypes=[("Map files", "*.map")])
        if not path: return
        try:
            with open(path, "r", encoding="utf-8") as f: lines = f.readlines()
            self.wave_list.delete(0, tk.END)
            mode = ""; row_idx = 0
            for line in lines:
                line = line.strip()
                if not line: continue
                if "width=" in line: self.width = int(line.split("=")[1])
                elif "height=" in line: self.height = int(line.split("=")[1])
                elif "money=" in line: self.ent_money.delete(0, tk.END); self.ent_money.insert(0, line.split("=")[1])
                elif "name=" in line: self.ent_name.delete(0, tk.END); self.ent_name.insert(0, line.split("=")[1])
                elif "tiles=" in line:
                    mode = "tiles"; self.tiles = [[0 for _ in range(self.width)] for _ in range(self.height)]; row_idx = 0
                elif "waves=" in line: mode = "waves"
                elif mode == "tiles":
                    nums = list(map(int, line.split()))
                    for x, v in enumerate(nums):
                        if x < self.width: self.tiles[row_idx][x] = v
                    row_idx += 1
                elif mode == "waves": self.wave_list.insert(tk.END, line)
            self.ent_w.delete(0, tk.END); self.ent_w.insert(0, str(self.width))
            self.ent_h.delete(0, tk.END); self.ent_h.insert(0, str(self.height))
            self.refresh_canvas()
        except Exception as e: messagebox.showerror("Error", str(e))

if __name__ == "__main__":
    root = tk.Tk()
    app = LevelEditor(root)
    root.mainloop()
