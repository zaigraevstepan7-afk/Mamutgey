-- [[ TABLET FLY LEFT ALIGN by koko ]]
-- Кнопки смещены влево
-- Платформа не падает вниз
-- Скорость домой: 25

local Players = game:GetService("Players")
local LocalPlayer = Players.LocalPlayer
local UserInputService = game:GetService("UserInputService")
local RunService = game:GetService("RunService")
local TweenService = game:GetService("TweenService")

-- Настройки
local FlySpeed = 23
local FlyEnabled = false
local PlatformEnabled = false
local BasePosition = nil
local GoingToBase = false
local LastSafeY = nil
local GoHomeSpeed = 25  -- Скорость движения домой

-- Проверка на планшет
local isTablet = UserInputService.TouchEnabled

-- Простая проверка
local function SafeCheck()
    if not LocalPlayer or not LocalPlayer.Character then return false end
    local humanoid = LocalPlayer.Character:FindFirstChild("Humanoid")
    local rootPart = LocalPlayer.Character:FindFirstChild("HumanoidRootPart")
    return humanoid and rootPart and humanoid.Health > 0
end

-- Основной GUI
local ScreenGui = Instance.new("ScreenGui")
ScreenGui.Name = "TabletFlyLeft"
ScreenGui.Parent = game:GetService("CoreGui")

-- Плавающая кнопка для открытия меню
local OpenButton = Instance.new("TextButton")
OpenButton.Size = UDim2.new(0, 80, 0, 80)
OpenButton.Position = UDim2.new(1, -90, 0, 10)
OpenButton.AnchorPoint = Vector2.new(1, 0)
OpenButton.Text = "🎮"
OpenButton.TextColor3 = Color3.new(1, 1, 1)
OpenButton.BackgroundColor3 = Color3.fromRGB(0, 100, 200)
OpenButton.TextSize = 30
OpenButton.Visible = true

local OpenButtonCorner = Instance.new("UICorner")
OpenButtonCorner.CornerRadius = UDim.new(0, 40)
OpenButtonCorner.Parent = OpenButton

OpenButton.Parent = ScreenGui

-- Главное меню
local MainFrame = Instance.new("Frame")
MainFrame.Size = UDim2.new(0, 320, 0, 450)
MainFrame.Position = UDim2.new(0.5, -160, 0.5, -225)
MainFrame.AnchorPoint = Vector2.new(0.5, 0.5)
MainFrame.BackgroundColor3 = Color3.fromRGB(20, 20, 30)
MainFrame.BackgroundTransparency = 0.1
MainFrame.BorderSizePixel = 0
MainFrame.Visible = false
MainFrame.Active = true
MainFrame.Draggable = true

local MainCorner = Instance.new("UICorner")
MainCorner.CornerRadius = UDim.new(0, 20)
MainCorner.Parent = MainFrame

local MainStroke = Instance.new("UIStroke")
MainStroke.Color = Color3.fromRGB(0, 150, 255)
MainStroke.Thickness = 3
MainStroke.Parent = MainFrame

MainFrame.Parent = ScreenGui

-- Заголовок меню
local Title = Instance.new("TextLabel")
Title.Size = UDim2.new(1, 0, 0, 80)
Title.Position = UDim2.new(0, 0, 0, 0)
Title.Text = "TABLET FLY\nby koko"
Title.TextColor3 = Color3.new(1, 1, 1)
Title.BackgroundColor3 = Color3.fromRGB(0, 80, 160)
Title.TextScaled = true
Title.Font = Enum.Font.GothamBold
Title.Parent = MainFrame

local TitleCorner = Instance.new("UICorner")
TitleCorner.CornerRadius = UDim.new(0, 20)
TitleCorner.Parent = Title

-- Кнопка закрытия меню
local CloseButton = Instance.new("TextButton")
CloseButton.Size = UDim2.new(0, 50, 0, 50)
CloseButton.Position = UDim2.new(1, -60, 0, 15)
CloseButton.Text = "✕"
CloseButton.TextColor3 = Color3.new(1, 1, 1)
CloseButton.BackgroundColor3 = Color3.fromRGB(200, 0, 0)
CloseButton.TextSize = 20
CloseButton.Font = Enum.Font.GothamBold

local CloseCorner = Instance.new("UICorner")
CloseCorner.CornerRadius = UDim.new(0, 25)
CloseCorner.Parent = CloseButton

CloseButton.Parent = MainFrame

-- Функция создания кнопок СЛЕВА
local function CreateLeftButton(text, yPosition, icon)
    local button = Instance.new("TextButton")
    button.Size = UDim2.new(0, 280, 0, 70)
    button.Position = UDim2.new(0, 20, 0, yPosition)
    button.Text = icon .. " " .. text
    button.TextColor3 = Color3.new(1, 1, 1)
    button.BackgroundColor3 = Color3.fromRGB(40, 40, 60)
    button.TextSize = 18
    button.Font = Enum.Font.Gotham
    button.AutoButtonColor = true
    button.TextXAlignment = Enum.TextXAlignment.Left
    
    local buttonCorner = Instance.new("UICorner")
    buttonCorner.CornerRadius = UDim.new(0, 15)
    buttonCorner.Parent = button
    
    local buttonStroke = Instance.new("UIStroke")
    buttonStroke.Color = Color3.fromRGB(100, 100, 150)
    buttonStroke.Thickness = 2
    buttonStroke.Parent = button
    
    local buttonPadding = Instance.new("UIPadding")
    buttonPadding.PaddingLeft = UDim.new(0, 15)
    buttonPadding.Parent = button
    
    button.Parent = MainFrame
    return button
end

-- Создаем кнопки СЛЕВА
local FlyButton = CreateLeftButton("FLY UP", 90, "🚀")
local PlatformButton = CreateLeftButton("PLATFORM", 170, "🏗️")
local SetBaseButton = CreateLeftButton("SET HOME", 250, "📍")
local GoBaseButton = CreateLeftButton("GO HOME", 330, "🏠")

-- Статус бар тоже слева
local StatusBar = Instance.new("Frame")
StatusBar.Size = UDim2.new(1, -40, 0, 50)
StatusBar.Position = UDim2.new(0, 20, 1, -60)
StatusBar.BackgroundColor3 = Color3.fromRGB(30, 30, 50)
StatusBar.BorderSizePixel = 0

local StatusCorner = Instance.new("UICorner")
StatusCorner.CornerRadius = UDim.new(0, 10)
StatusCorner.Parent = StatusBar

StatusBar.Parent = MainFrame

local StatusLabel = Instance.new("TextLabel")
StatusLabel.Size = UDim2.new(1, 0, 1, 0)
StatusLabel.Position = UDim2.new(0, 0, 0, 0)
StatusLabel.Text = "Ready to fly!"
StatusLabel.TextColor3 = Color3.new(1, 1, 1)
StatusLabel.BackgroundTransparency = 1
StatusLabel.TextXAlignment = Enum.TextXAlignment.Left
StatusLabel.Font = Enum.Font.Gotham
StatusLabel.TextSize = 16

local StatusPadding = Instance.new("UIPadding")
StatusPadding.PaddingLeft = UDim.new(0, 10)
StatusPadding.Parent = StatusLabel

StatusLabel.Parent = StatusBar

-- Переменные
local flyConnection = nil
local platformConnection = nil
local baseConnection = nil
local currentPlatform = nil

-- Функция полета
local function SimpleFly()
    if not SafeCheck() then return end
    local rootPart = LocalPlayer.Character:FindFirstChild("HumanoidRootPart")
    rootPart.Velocity = Vector3.new(0, FlySpeed, 0)
end

-- Функция платформы с защитой от падения
local function CreatePlatform()
    if not SafeCheck() then return nil end
    
    if currentPlatform then
        currentPlatform:Destroy()
    end
    
    local rootPart = LocalPlayer.Character:FindFirstChild("HumanoidRootPart")
    local platform = Instance.new("Part")
    platform.Name = "TabletPlatform"
    platform.Size = Vector3.new(12, 2, 12)
    platform.Position = rootPart.Position - Vector3.new(0, 5, 0)
    platform.Anchored = true
    platform.CanCollide = true
    platform.Transparency = 0.3
    platform.BrickColor = BrickColor.new("Bright blue")
    platform.Material = Enum.Material.Neon
    platform.Parent = workspace
    
    LastSafeY = platform.Position.Y
    
    return platform
end

local function UpdatePlatform()
    if not PlatformEnabled or not currentPlatform or not SafeCheck() then return end
    
    local rootPart = LocalPlayer.Character:FindFirstChild("HumanoidRootPart")
    local newPosition = Vector3.new(rootPart.Position.X, rootPart.Position.Y - 5, rootPart.Position.Z)
    
    if LastSafeY and newPosition.Y < LastSafeY then
        newPosition = Vector3.new(newPosition.X, LastSafeY, newPosition.Z)
    else
        LastSafeY = newPosition.Y
    end
    
    currentPlatform.Position = newPosition
end

-- Функция движения домой со скоростью 25
local function GoToBase()
    if not BasePosition then
        StatusLabel.Text = "Set home first!"
        return
    end
    
    GoingToBase = true
    StatusLabel.Text = "Going home..."
    
    baseConnection = RunService.Heartbeat:Connect(function()
        if not GoingToBase or not SafeCheck() then
            if baseConnection then baseConnection:Disconnect() end
            return
        end
        
        local rootPart = LocalPlayer.Character:FindFirstChild("HumanoidRootPart")
        local direction = (BasePosition - rootPart.Position).Unit
        local distance = (BasePosition - rootPart.Position).Magnitude
        
        if distance < 5 then
            GoingToBase = false
            StatusLabel.Text = "Arrived home!"
            if baseConnection then baseConnection:Disconnect() end
        else
            -- ИЗМЕНЕНИЕ: скорость 25 вместо 50
            rootPart.Velocity = direction * GoHomeSpeed + Vector3.new(0, 3, 0)
        end
    end)
end

-- Анимация кнопок
local function AnimateButton(button, active)
    local targetColor = active and Color3.fromRGB(0, 150, 0) or Color3.fromRGB(40, 40, 60)
    local tweenInfo = TweenInfo.new(0.2, Enum.EasingStyle.Quad, Enum.EasingDirection.Out)
    local tween = TweenService:Create(button, tweenInfo, {BackgroundColor3 = targetColor})
    tween:Play()
end

-- Открытие/закрытие меню
OpenButton.MouseButton1Click:Connect(function()
    MainFrame.Visible = true
    OpenButton.Visible = false
end)

CloseButton.MouseButton1Click:Connect(function()
    MainFrame.Visible = false
    OpenButton.Visible = true
end)

-- Обработчики кнопок меню
FlyButton.MouseButton1Click:Connect(function()
    FlyEnabled = not FlyEnabled
    AnimateButton(FlyButton, FlyEnabled)
    
    if FlyEnabled then
        FlyButton.Text = "✅ FLY: ON"
        StatusLabel.Text = "Flying up!"
        
        flyConnection = RunService.Heartbeat:Connect(function()
            if not FlyEnabled then
                flyConnection:Disconnect()
                return
            end
            SimpleFly()
        end)
    else
        FlyButton.Text = "🚀 FLY UP"
        StatusLabel.Text = "Fly stopped"
        
        if flyConnection then
            flyConnection:Disconnect()
        end
    end
end)

PlatformButton.MouseButton1Click:Connect(function()
    PlatformEnabled = not PlatformEnabled
    AnimateButton(PlatformButton, PlatformEnabled)
    
    if PlatformEnabled then
        PlatformButton.Text = "✅ PLATFORM: ON"
        StatusLabel.Text = "Platform created (no fall)"
        
        currentPlatform = CreatePlatform()
        platformConnection = RunService.Heartbeat:Connect(UpdatePlatform)
    else
        PlatformButton.Text = "🏗️ PLATFORM"
        StatusLabel.Text = "Platform removed"
        
        if platformConnection then
            platformConnection:Disconnect()
        end
        if currentPlatform then
            currentPlatform:Destroy()
            currentPlatform = nil
        end
        LastSafeY = nil
    end
end)

SetBaseButton.MouseButton1Click:Connect(function()
    if SafeCheck() then
        BasePosition = LocalPlayer.Character:FindFirstChild("HumanoidRootPart").Position
        StatusLabel.Text = "Home position saved!"
        SetBaseButton.Text = "📍 HOME SET!"
        
        wait(1)
        SetBaseButton.Text = "📍 SET HOME"
    end
end)

GoBaseButton.MouseButton1Click:Connect(function()
    if GoingToBase then
        GoingToBase = false
        StatusLabel.Text = "Cancelled"
        GoBaseButton.Text = "🏠 GO HOME"
    else
        GoBaseButton.Text = "✅ GOING HOME..."
        GoToBase()
    end
end)

-- Автоочистка при респавне
LocalPlayer.CharacterAdded:Connect(function()
    wait(2)
    
    if flyConnection then flyConnection:Disconnect() end
    if platformConnection then platformConnection:Disconnect() end
    if baseConnection then baseConnection:Disconnect() end
    if currentPlatform then currentPlatform:Destroy() end
    
    FlyEnabled = false
    PlatformEnabled = false
    GoingToBase = false
    LastSafeY = nil
    
    FlyButton.Text = "🚀 FLY UP"
    PlatformButton.Text = "🏗️ PLATFORM"
    SetBaseButton.Text = "📍 SET HOME"
    GoBaseButton.Text = "🏠 GO HOME"
    StatusLabel.Text = "Respawned - Ready!"
    
    AnimateButton(FlyButton, false)
    AnimateButton(PlatformButton, false)
end)

-- Горячие клавиши для ПК
if not isTablet then
    UserInputService.InputBegan:Connect(function(input)
        if input.KeyCode == Enum.KeyCode.H then
            MainFrame.Visible = not MainFrame.Visible
            OpenButton.Visible = not MainFrame.Visible
        end
    end)
end

print("📱 Tablet Fly LEFT ALIGN by koko loaded")
print("⬅️ Buttons aligned to left")
print("🛡️ Platform cannot go down - fall protection enabled")
print("🏠 Go home speed: " .. GoHomeSpeed)
print("📏 Position: 20 pixels from left edge")
