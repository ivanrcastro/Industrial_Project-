# Usar a imagem desktop para ter as ferramentas de simulação (ex:Gazebo)
FROM osrf/ros:humble-desktop

# Evitar perguntas interativas durante a instalação
ENV DEBIAN_FRONTEND=noninteractive

# 1. Instalar dependências essenciais do sistema e ferramentas de build
RUN apt-get update && apt-get install -y \
    python3-colcon-common-extensions \
    python3-rosdep \
    python3-pip \
    build-essential \
    cmake \
    git \
    wget \
    # Dependências para MQTT em C++
    libpaho-mqtt-dev \
    libpaho-mqttpp-dev \
    # Dependências de Robótica e Simulação
    ros-humble-navigation2 \
    ros-humble-nav2-bringup \
    ros-humble-turtlebot3-gazebo \
    ros-humble-rmw-cyclonedds-cpp \
    && rm -rf /var/lib/apt/lists/*

# 2. Configurar o Workspace de trabalho
WORKDIR /ros2_ws

# 3. Criar um script de "entrypoint" para carregar o ambiente ROS automaticamente
RUN echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc

# 4. Variável para garantir estabilidade na comunicação (CycloneDDS)
ENV RMW_IMPLEMENTATION=rmw_cyclonedds_cpp

# O container inicia em modo interativo por defeito
CMD ["/bin/bash"]